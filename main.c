/*
 * main.c --- Main file.
 *
 * Copyright (c) 2016 Paul Ward <asmodai@gmail.com>
 *
 * Author:     Paul Ward <asmodai@gmail.com>
 * Maintainer: Paul Ward <asmodai@gmail.com>
 * Created:    23 Dec 2016 14:47:18
 */
/* {{{ License: */
/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are  permitted provided that the following conditions are
 * met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer. 
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/* }}} */
/* {{{ Commentary: */
/*
 *
 */
/* }}} */

/**
 * @file main.c
 * @author Paul Ward
 * @brief Main file.
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <time.h>

#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# include "posix/stdbool.h"
#endif

#include "json.h"
#include "endpoints.h"
#include "timers.h"
#include "fdwatch.h"
#include "httpd.h"
#include "utils.h"

#include "sm_uname.h"
#include "sm_smver.h"
#include "sm_info.h"
#include "sm_cpu.h"
#include "sm_all.h"

#if PLATFORM_EQ(PLATFORM_BSD)
# if PLATFORM_LT(PLATFORM_BSD, PLATFORM_BSDOS)
#  include <sys/uio.h>
extern gettimeofday();
extern free();
extern fprintf();
extern syslog();
extern read();
extern write();
extern writev();
extern strerror();
extern time();
extern bcopy();
extern bzero();
extern memset();
typedef signed long ssize_t;
#  define memmove(__d, __s, __n)  bcopy((__s), (__d), (__n))
#  ifndef EXIT_FAILURE
#   define EXIT_FAILURE 1
#  endif
#  if PLATFORM_EQ(PLATFORM_ULTRIX)
extern void *malloc(size_t);
extern void *realloc(void *, size_t);
#  else
extern char *malloc();
extern char *realloc();
#  endif
# endif
#endif

typedef struct {
  int           state;
  int           next_free_connect;
  http_conn_t  *conn;
  time_t        started;
  time_t        active;
  timer_task_t *wakeup;
  long          wouldblock_delay;
  off_t         bytes;
  off_t         end_byte_idx;
  off_t         next_byte_idx;
} connect_t;

#define CNST_FREE      0
#define CNST_READING   1
#define CNST_SENDING   2
#define CNST_PAUSING   3

static httpd_t    *server             = NULL;
static connect_t  *connects           = NULL;
static int         num_connects       = 0;
static int         max_connects       = 0;
static int         first_free_connect = 0;
static int         httpd_conn_count   = 0;

time_t start_time         = 0;
time_t stats_time         = 0;
int    terminate          = 0;
off_t  stats_bytes        = 0;
long   stats_connections  = 0;
int    stats_simultaneous = 0;

static void finish_connection(connect_t *, struct timeval *);
static void clear_connection(connect_t *, struct timeval *);

void
terminate_app(void)
{
  terminate = 1;
}

static
void
shut_down(void)
{
  size_t         cnum = 0;
  struct timeval tv;

  gettimeofday(&tv, NULL);
  //logstats(&tv);
  
  for (cnum = 0; cnum < max_connects; cnum++) {
    if (connects[cnum].conn != NULL) {
      httpd_close_conn(connects[cnum].conn);
      httpd_destroy_conn(connects[cnum].conn);
      --httpd_conn_count;
      connects[cnum].conn = NULL;
    }
  }

  if (server != NULL) {
    httpd_t *ptr = server;
    server       = NULL;

    httpd_terminate(ptr);
    ptr = NULL;
  }

  tmr_term();

  free(connects);
}

static
void
idle(timer_clientdata_t data, struct timeval *tv)
{
  size_t     cnum = 0;
  connect_t *conn;

  extern void httpd_derp_stats();

#ifdef DEBUG
  fprintf(stderr, "TIMER FIRE - Occasional\n");
#endif

  for (cnum = 0; cnum < max_connects; cnum++) {
    conn = &connects[cnum];

    switch (conn->state) {
      case CNST_READING:
        if (tv->tv_sec - conn->active >= IDLE_READ_TIMELIMIT) {
#ifdef DEBUG
          fprintf(stderr, "TIMING OUT - %.80s [read]\n",
                  httpd_ntoa(&conn->conn->client_addr));
#endif
          syslog(LOG_INFO, "%.80s connection timed out whist reading",
                 httpd_ntoa(&conn->conn->client_addr));
          httpd_send_err(conn->conn,
                         408,
                         err408title,
                         "",
                         err408form);
          finish_connection(conn, tv);
        }
        break;

      case CNST_SENDING:
      case CNST_PAUSING:
        if (tv->tv_sec - conn->active >= IDLE_SEND_TIMELIMIT) {
#ifdef DEBUG
          fprintf(stderr, "TIMING OUT - %.80s [write]\n",
                  httpd_ntoa(&conn->conn->client_addr));
#endif

          syslog(LOG_INFO, "%.80s connection timed out whilst sending",
                 httpd_ntoa(&conn->conn->client_addr));
          clear_connection(conn, tv);
        }
        break;
    }
  }
}

static
void
wakeup_connection(timer_clientdata_t data, struct timeval *tv)
{
  connect_t *conn = (connect_t *)data.p;

#ifdef DEBUG
  fprintf(stderr, "TIMER FIRE - Wakeup connection");
#endif

  conn->wakeup = NULL;
  if (conn->state == CNST_PAUSING) {
    conn->state = CNST_SENDING;
    fdwatch_add_fd(conn->conn->conn_fd, conn, FDW_WRITE);
  }
}

static
void
really_clear_connection(connect_t *conn, struct timeval *tv)
{
  stats_bytes += conn->conn->bytes_sent;

  if (conn->state != CNST_PAUSING) {
    fdwatch_del_fd(conn->conn->conn_fd);
  }

  httpd_close_conn(conn->conn);
  httpd_destroy_conn(conn->conn);

  conn->state             = CNST_FREE;
  conn->next_free_connect = first_free_connect;
  first_free_connect      = conn - connects;
  --num_connects;
}

static
void
clear_connection(connect_t *conn, struct timeval *tv)
{
  if (conn->wakeup != NULL) {
    tmr_cancel(conn->wakeup);
    conn->wakeup = NULL;
  }

  really_clear_connection(conn, tv);
}

static
void
finish_connection(connect_t *conn, struct timeval *tv)
{
  httpd_write_response(conn->conn);
  clear_connection(conn, tv);
}

static
int
handle_newconnect(struct timeval *tv, int fd)
{
  connect_t *conn = NULL;

  for (;;) {
    if (num_connects >= max_connects) {
      syslog(LOG_WARNING, "Too many connections!");
      tmr_run(tv);
      return 0;
    }

    if (first_free_connect                 == -1 ||
        connects[first_free_connect].state != CNST_FREE)
    {
      syslog(LOG_CRIT, "The connections free list is messed up");
      exit(EXIT_FAILURE);
    }

    conn = &connects[first_free_connect];
    if (conn->conn == NULL) {
      conn->conn = (http_conn_t *)malloc(sizeof(http_conn_t));
      if (conn->conn == NULL) {
        syslog(LOG_CRIT, "Out of memory allocating HTTP connection.");
        exit(EXIT_FAILURE);
      }

      conn->conn->initialised = 0;
      ++httpd_conn_count;
    }

    switch (httpd_get_conn(server, fd, conn->conn)) {
      case GC_FAIL:
        tmr_run(tv);
        return 0;

      case GC_NO_MORE:
        return 1;
    }

    conn->state             = CNST_READING;
    first_free_connect      = conn->next_free_connect;
    conn->next_free_connect = -1;
    ++num_connects;
    conn->active            = tv->tv_sec;
    conn->wakeup            = NULL;
    conn->next_byte_idx     = 0;

    httpd_set_ndelay(conn->conn->conn_fd);
    fdwatch_add_fd(conn->conn->conn_fd, conn, FDW_READ);

    ++stats_connections;
    if (num_connects > stats_connections) {
      stats_connections = num_connects;
    }
  }
}

static
void
handle_read(connect_t *conn, struct timeval *tv)
{
  int          sz    = -1;
  http_conn_t *hconn = conn->conn;

  if (hconn->read_idx >= hconn->read_size) {
    if (hconn->read_size > 5000) {
      httpd_send_err(hconn,
                     400,
                     err400title,
                     "",
                     err400form);
      finish_connection(conn, tv);
      return;
    }

    httpd_realloc_str(&hconn->read_buf,
                      &hconn->read_size,
                      hconn->read_size + 128);
  }

  sz = read(hconn->conn_fd,
            &(hconn->read_buf[hconn->read_idx]),
            hconn->read_size - hconn->read_idx);
  if (sz == 0) {
    httpd_send_err(hconn,
                   400,
                   err400title,
                   "",
                   err400form);
    finish_connection(conn, tv);
    return;
  }

  if (sz < 0) {
    if (errno == EINTR  ||
        errno == EAGAIN ||
        errno == EWOULDBLOCK)
    {
      return;
    }

    httpd_send_err(hconn,
                   400,
                   err400title,
                   "",
                   err400form);
    finish_connection(conn, tv);
    return;
  }

  hconn->read_idx += sz;
  conn->active     = tv->tv_sec;

  switch (httpd_got_request(hconn)) {
    case GR_NO_REQUEST:
      return;
    case GR_BAD_REQUEST:
      httpd_send_err(hconn,
                     400,
                     err400title,
                     "",
                     err400form);
      finish_connection(conn, tv);
      return;
  }

  if (httpd_parse_request(hconn) < 0) {
    finish_connection(conn, tv);
    return;
  }

  if (httpd_start_request(hconn, tv) < 0) {
    finish_connection(conn, tv);
    return;
  }

  /* We don't do byte ranges... */
  if (hconn->bytes_to_send < 0) {
    conn->end_byte_idx = 0;
  } else {
    conn->end_byte_idx = hconn->bytes_to_send;
  }

  if (conn->next_byte_idx >= conn->end_byte_idx) {
    finish_connection(conn, tv);
    return;
  }

  conn->state            = CNST_SENDING;
  conn->started          = tv->tv_sec;
  conn->wouldblock_delay = 0;
  
  fdwatch_del_fd(hconn->conn_fd);
  fdwatch_add_fd(hconn->conn_fd, conn, FDW_WRITE);
}

static
void
handle_send(connect_t *conn, struct timeval *tv)
{
  ssize_t             max_bytes = 100000L;
  ssize_t             sz        = 0;
  timer_clientdata_t  cd        = JunkClientData;
  http_conn_t        *hconn     = conn->conn;
  
  if (hconn->response_len == 0) {
    sz = write(hconn->conn_fd,
               &(hconn->data_address[conn->next_byte_idx]),
               MIN(conn->end_byte_idx - conn->next_byte_idx, max_bytes));
  } else {
    struct iovec iv[2];

    iv[0].iov_base = hconn->response;
    iv[0].iov_len  = hconn->response_len;
    iv[1].iov_base = &(hconn->data_address[conn->next_byte_idx]);
    iv[1].iov_len  = MIN(conn->end_byte_idx - conn->next_byte_idx, max_bytes);

    sz = writev(hconn->conn_fd, iv, 2);
  }

  if (sz < 0 && errno == EINTR) {
    return;
  }

  if (sz == 0 ||
      (sz < 0 && (errno  = EWOULDBLOCK || errno == EAGAIN)))
  {
    conn->wouldblock_delay += MIN_WOULDBLOCK_DELAY;
    conn->state             = CNST_PAUSING;
    fdwatch_del_fd(hconn->conn_fd);
    cd.p                    = conn;

    if (conn->wakeup != NULL) {
      syslog(LOG_ERR, "Replacing non-NULL wakeup timer!");
    }

    conn->wakeup = tmr_create(tv,
                              wakeup_connection,
                              cd,
                              conn->wouldblock_delay,
                              0);
    if (conn->wakeup == NULL) {
      syslog(LOG_CRIT, "Could not create wakeup timer");
      exit(EXIT_FAILURE);
    }

    return;
  }

  if (sz < 0) {
    if (errno != EPIPE && errno != EINVAL && errno != ECONNRESET) {
      syslog(LOG_ERR, "write - %s sending %.80s",
             strerror(errno),
             hconn->encoded_url);
    }

    clear_connection(conn, tv);
    return;
  }

  conn->active = tv->tv_sec;

  if (hconn->response_len > 0) {
    if ((size_t)sz < hconn->response_len) {
      size_t newlen = hconn->response_len - (size_t)sz;

      memmove(hconn->response, &(hconn->response[sz]), newlen);
      hconn->response_len = newlen;
      sz                  = 0;
    } else {
      sz                  -= hconn->response_len;
      hconn->response_len  = 0;
    }
  }

  conn->next_byte_idx    += sz;
  conn->conn->bytes_sent += sz;

  if (conn->next_byte_idx >= conn->end_byte_idx) {
    finish_connection(conn, tv);
    return;
  }

  if (conn->wouldblock_delay > MIN_WOULDBLOCK_DELAY) {
    conn->wouldblock_delay -= MIN_WOULDBLOCK_DELAY;
  }
}

#ifdef DEBUG
void
dump_data(void)
{
  size_t cnum  = 0;
  size_t nulls = 0;

  fprintf(stderr, "Connection table:\n");
  for (cnum = 0; cnum < max_connects; cnum++) {
    if (connects[cnum].conn != NULL) {
      fprintf(stderr, "   %06ld -  ", cnum);
      fprintf(stderr, "state:%d read_size:%ld response_len:%ld\n",
              connects[cnum].state,
              connects[cnum].conn->read_size,
              connects[cnum].conn->response_len);

      if (connects[cnum].conn->read_buf != NULL) {
        fprintf(stderr, "    read_buf:LIAR [%s]\n",
                connects[cnum].conn->read_buf);
      }

      if (connects[cnum].conn->response != NULL) {
        fprintf(stderr, "    response:LIAR [%s]\n",
                connects[cnum].conn->response);
      }
    } else {
      nulls++;
    }
  }
  fprintf(stderr, "%ld NULL entries.\n", nulls);
}
#endif

int
main(void)
{
  connect_t      *conn      = NULL;
  http_conn_t    *hconn     = NULL;
  struct timeval  tv;
  int             num_ready = 0;
  int             cnum      = 0;
  sockaddr_t addr;

  /*
    fclose(stdin);
    fclose(stdout);
    fclose(stderr);
  */

  max_connects = MIN(fdwatch_get_nfiles(), 8);
  if (max_connects <= 0) {
    syslog(LOG_CRIT, "fdwatch initialisation failure");
    exit(EXIT_FAILURE);
  }

/*
  max_connects -= SPARE_FDS;
 */

  tmr_init();
  endpoint_init();

  sm_uname_init();
  sm_smver_init();
  sm_info_init();
  sm_cpu_init();
  sm_all_init();

  addr.sa_in.sin_family      = AF_INET;
  addr.sa_in.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sa_in.sin_port        = htons(HTTPD_PORT);

  server = httpd_init(&addr, -1);
  if (server == NULL) {
    exit(EXIT_FAILURE);
  }

  if (tmr_create(NULL,
                 idle,
                 JunkClientData,
                 5000L,
                 1) == NULL)
  {
    syslog(LOG_CRIT, "Could not create occasional timer.");
    exit(EXIT_FAILURE);
  }

  start_time         = stats_time = time(NULL);
  stats_connections  = 0;
  stats_bytes        = 0;
  stats_simultaneous = 0;

  connects = malloc(sizeof(connect_t) * max_connects);
  if (connects == NULL) {
    syslog(LOG_CRIT, "Could not allocate memory for connection table");
    exit(EXIT_FAILURE);
  }
  for (cnum = 0; cnum < max_connects; cnum++) {
    connects[cnum].state             = CNST_FREE;
    connects[cnum].next_free_connect = cnum + 1;
    connects[cnum].conn              = NULL;
  }
  /* -1 */
  connects[max_connects - 1].next_free_connect = -1;
  first_free_connect                           = 0;
  num_connects                                 = 0;
  httpd_conn_count                             = 0;

  if (server != NULL) {
    fdwatch_add_fd(server->listen_fd, NULL, FDW_READ);
  }

  gettimeofday(&tv, NULL);
  while ((!terminate) || (num_connects > 0)) {
    num_ready = fdwatch(tmr_mstimeout(&tv));
    if (num_ready < 0) {
      if (errno == EINTR || errno == EAGAIN) {
        continue;
      }

      syslog(LOG_ERR, "fdwatch - %s", strerror(errno));
      exit(EXIT_FAILURE);
    }

    gettimeofday(&tv, NULL);
    if (num_ready == 0) {
      tmr_run(&tv);
      continue;
    }

    if (server            != NULL &&
        server->listen_fd != -1 &&
        fdwatch_check_fd(server->listen_fd))
    {
      if (handle_newconnect(&tv, server->listen_fd)) {
        continue;
      }
    }

    while ((conn = (connect_t *)fdwatch_get_next_client_data())
           != (connect_t *)-1)
    {
      if (conn == NULL) {
        continue;
      }
      
      hconn = conn->conn;
      if (!fdwatch_check_fd(hconn->conn_fd)) {
        clear_connection(conn, &tv);
      } else {
        switch (conn->state) {
          case CNST_READING:   handle_read(conn, &tv);    break;
          case CNST_SENDING:   handle_send(conn, &tv);    break;
        }
      }
    }
    tmr_run(&tv);
  }

  shut_down();
  syslog(LOG_NOTICE, "Exiting.");

  return 0;
}

/* main.c ends here. */
