/*
 * httpd.c --- HTTP server.
 *
 * Copyright (c) 2017 Paul Ward <asmodai@gmail.com>
 *
 * Author:     Paul Ward <asmodai@gmail.com>
 * Maintainer: Paul Ward <asmodai@gmail.com>
 * Created:    19 Mar 2017 07:49:28
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
 * @file httpd.c
 * @author Paul Ward
 * @brief HTTP server.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <syslog.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "config.h"
#include "version.h"
#include "httpd.h"
#include "utils.h"
#include "endpoints.h"
#include "json.h"
#include "sm_all.h"

static size_t str_alloc_count = 0;
static size_t str_alloc_size  = 0;

const char *proto09 = "HTTP/0.9";
const char *proto10 = "HTTP/1.0";
const char *proto11 = "HTTP/1.1";

char *ok200title = "OK";

char *err400title = "Bad Request";
char *err400form  = "Unable to satisfy your request.";
char *err404title = "Not Found";
char *err404form  = "The specified resource was not found.";
char *err408title = "Request Timeout";
char *err408form  = "No request appeared within a reasonable time period.";
char *err500title = "Internal Server Error";
char *err500form  = "The server encountered an error, check logs.";
char *err501title = "Not Implemented";
char *err501form  = "The requested method is not implemented.";
char *err503title = "Server Temporarily Overloaded";
char *err503form  = "Please try again later.";

static
char *
bufgets(http_conn_t *conn)
{
  size_t i  = 0;
  char   ch = 0;

  for (i = conn->checked_idx;
       conn->checked_idx < conn->read_idx;
       conn->checked_idx++)
  {
    ch = conn->read_buf[conn->checked_idx];

    if (ch == '\012' || ch == '\015') {
      conn->read_buf[conn->checked_idx] = '\0';
      ++conn->checked_idx;

      if (ch                                == '\015'         &&
          conn->checked_idx                  < conn->read_idx &&
          conn->read_buf[conn->checked_idx] == '\012')
      {
        conn->read_buf[conn->checked_idx] = '\0';
        ++conn->checked_idx;
      }
      
      return &(conn->read_buf[i]);
    }
  }

  return NULL;
}

#ifndef ol_strcpy
# define ol_strcpy(__d, __s)    memmove((__d), (__s), strlen((__s) - 1))
#endif

static
void
de_dotdot(char *path)
{
  char *cp1 = NULL;
  char   *cp2 = NULL;
  size_t l = 0;

  while ((cp1 = strstr(path, "//")) != NULL) {
    for (cp2 = cp1 + 2; *cp2 == '/'; cp2++) {
      continue;
    }

    ol_strcpy(cp1 + 1, cp2);
  }

  while (strncmp(path, "./", 2) == 0) {
    ol_strcpy(path, path + 2);
  }

  while ((cp1 = strstr(path, "/./")) != NULL) {
    ol_strcpy(cp1, cp1 + 2);
  }

  for (;;) {
    while (strncmp(path, "../", 3) == 0) {
      ol_strcpy(path, path + 3);
    }

    cp1 = strstr(path, "/../");
    if (cp1 == NULL) {
      break;
    }

    for (cp2 = cp1 - 1; cp2 >= path && *cp2 != '/'; cp2--) {
      continue;
    }

    ol_strcpy(cp2 + 1, cp1 + 4);
  }

  while ((l = strlen(path)) > 3 &&
         strcmp((cp1 = path + l - 3), "/..") == 0)
  {
    for (cp2 = cp1 - 1; cp2 >= path && *cp2 != '/'; cp2--) {
      continue;
    }

    if (cp2 < path) {
      break;
    }

    *cp2 = '\0';
  }
}

void
httpd_realloc_str(char **ptr, size_t *max, size_t sz)
{
  if (*max == 0) {
    *max            = MAX(200, sz + 1);
    *ptr            = malloc(sizeof(char) * *max + 1);
    ++str_alloc_count;
    str_alloc_size += *max;
  } else if (sz > *max) {
    str_alloc_size -= *max;
    *max            = MAX(*max * 2, sz * 5 / 4);
    *ptr            = realloc(*ptr, sizeof(char) * *max + 1);
    str_alloc_size += *max;
  } else {
    return;
  }

  if (*ptr == NULL) {
    syslog(LOG_ERR, "Out of memory reallocating string to %ld bytes.",
           (long)*max);
    exit(EXIT_FAILURE);
  }
}

static
void
httpd_free(httpd_t *hs)
{
  free(hs);
}

static
int
sockaddr_check(sockaddr_t *addr)
{
  switch (addr->sa.sa_family) {
    case AF_INET:  return 1;
    default:       return 0;
  }
}

static
size_t
sockaddr_len(sockaddr_t *addr)
{
  switch (addr->sa.sa_family) {
    case AF_INET: return sizeof(struct sockaddr_in);
    default:      return 0;
  }
}

char *
httpd_ntoa(sockaddr_t *addr)
{
  return inet_ntoa(addr->sa_in.sin_addr);
}

static
int
init_listen_sock(sockaddr_t *addr)
{
  int listen_fd = -1;
  int on        = -1;
  int flags = -1;

  if (!sockaddr_check(addr)) {
    syslog(LOG_CRIT, "Unknown address family for listen socket.");
    return -1;
  }

  listen_fd = socket(addr->sa.sa_family, SOCK_STREAM, 0);
  if (listen_fd < 0) {
    syslog(LOG_CRIT, "socket %.80s - %s",
           httpd_ntoa(addr),
           strerror(errno));
    return -1;
  }

  fcntl(listen_fd, F_SETFD, 1);
  on = 1;
  if (setsockopt(listen_fd,
                 SOL_SOCKET,
                 SO_REUSEADDR,
                 (char *)&on,
                 sizeof(on)) < 0)
  {
    syslog(LOG_CRIT, "setsockopt SO_REUSEADDR - %s",
           strerror(errno));
  }

  if (bind(listen_fd, &addr->sa, sockaddr_len(addr)) < 0) {
    syslog(LOG_CRIT, "bind %.80s - %s",
           httpd_ntoa(addr),
           strerror(errno));
    close(listen_fd);
    return -1;
  }

  flags = fcntl(listen_fd, F_GETFL, 0);
  if (flags == -1) {
    syslog(LOG_CRIT, "fcntl F_GETFL - %s", strerror(errno));
    close(listen_fd);
    return -1;
  }

  if (fcntl(listen_fd, F_SETFL, flags | O_NDELAY) < 0) {
    syslog(LOG_CRIT, "fcntl O_NDELAY - %s", strerror(errno));
    close(listen_fd);
    return -1;
  }

  if (listen(listen_fd, LISTEN_BACKLOG) < 0) {
    syslog(LOG_CRIT, "listen - %s", strerror(errno));
    close(listen_fd);
    return -1;
  }

  printf("Listen FD = %d\n", listen_fd);

  return listen_fd;
}

static
void
add_response(http_conn_t *conn, char *str)
{
  size_t len = 0;

  len = strlen(str);
  httpd_realloc_str(&conn->response,
                    &conn->max_response,
                    conn->response_len + len);
  memmove(&(conn->response[conn->response_len]), str, len);
  conn->response_len += len;
}

static
void
send_mime(http_conn_t *conn,
          int          status,
          char        *title,
          char        *encodings,
          char        *extraheads,
          char        *type,
          off_t        length,
          time_t       mod)
{
  time_t      now;
  const char *rfc1123fmt  = "%a, %d %b %Y %H:%M:%S GMT";
  char        nowbuf[100] = {0};
  char        modbuf[100] = {0};
  char        buf[1000]   = {0};

  conn->status        = status;
  conn->bytes_to_send = length;

  if (conn->mime_flag) {
    now = time(NULL);
    if (mod == 0) {
      mod = now;
    }

    strftime(nowbuf, sizeof(nowbuf), rfc1123fmt, gmtime(&now));
    strftime(modbuf, sizeof(modbuf), rfc1123fmt, gmtime(&mod));
    snprintf(buf,
             sizeof(buf),
             "%.20s %d %s\015\012"
             "Date: %s\015\012"
             "Server: %s\015\012"
             "Content-Type: %s\015\012"
             "Last-Modified: %s\015\012"
             "Cache-Cache: no-cache,no-store\015\012"
             "Connection: close\015\012",
             "HTTP/1.1",
            status,
            title,
            nowbuf,
            SERVER_SOFTWARE,
            type,
            modbuf);

    add_response(conn, buf);

    if (length >= 0) {
      snprintf(buf, sizeof(buf), "Content-Length: %lld\015\012",
              (long long)length);
      add_response(conn, buf);
    }

    if (extraheads[0] != '\0') {
      add_response(conn, extraheads);
    }

    add_response(conn, "\015\012");
  }
}

static
void
send_response(http_conn_t *conn,
              int          status,
              char        *title,
              char        *extraheads,
              char        *form)
{
  json_node_t *obj = json_mkobject();
  char        *json       = NULL;

  send_mime(conn,
            status,
            title,
            "",
            extraheads, "application/json",
            -1,
            0);

  json_prepend_member(obj, "path",    json_mkstring(conn->decoded_url));
  json_prepend_member(obj, "content", json_mkstring(form));
  json_prepend_member(obj, "title",   json_mkstring(title));
  json_prepend_member(obj, "status",  json_mknumber(status));

  json = json_stringify(obj, NULL);
  add_response(conn, json);
}

httpd_t *
httpd_init(sockaddr_t *addr, int max_age)
{
  httpd_t *hs = NULL;

  if ((hs = malloc(sizeof(httpd_t))) == 0) {
    syslog(LOG_CRIT, "Out of memory allocating HTTP server");
    return NULL;
  }

  hs->max_age   = max_age;
  hs->listen_fd = init_listen_sock(addr);

  if (hs->listen_fd == -1) {
    syslog(LOG_WARNING, "Could not create listener.");
    httpd_free(hs);
    return NULL;
  }

  syslog(LOG_NOTICE, "Starting on %.80s, port %d, fd %d",
         httpd_ntoa(addr), HTTPD_PORT, hs->listen_fd);

  return hs;
}

void
httpd_set_ndelay(int fd)
{
  int flags  = -1;
  int nflags = -1;

  flags = fcntl(fd, F_GETFL, 0);
  if (flags != -1) {
    nflags = flags & ~(int)O_NDELAY;

    if (nflags != flags) {
      fcntl(fd, F_SETFL, nflags);
    }
  }
}

int
httpd_get_conn(httpd_t *hs, int fd, http_conn_t *conn)
{
  sockaddr_t sa;
  socklen_t  sz = 0;

  if (!conn->initialised) {
    conn->read_size       = 0;
    conn->max_response    = 0;
    conn->max_reqhost     = 0;
    conn->max_query       = 0;
    conn->max_path        = 0;
    conn->max_decoded_url = 0;

    printf("Allocating...\n");

    httpd_realloc_str(&conn->read_buf,    &conn->read_size,       500);
    httpd_realloc_str(&conn->decoded_url, &conn->max_decoded_url, 2);
    httpd_realloc_str(&conn->reqhost,     &conn->max_reqhost,     0);
    httpd_realloc_str(&conn->query,       &conn->max_query,       0);
    httpd_realloc_str(&conn->response,    &conn->max_response,    0);
    httpd_realloc_str(&conn->path,        &conn->max_path,        1);

    conn->initialised = 1;
  }

  sz            = sizeof(sa.sa);
  conn->conn_fd = accept(fd, &sa.sa, &sz);
  if (conn->conn_fd < 0) {
    if (errno == EWOULDBLOCK) {
      return GC_NO_MORE;
    }

    if (errno != ECONNABORTED) {
      syslog(LOG_ERR, "accept - %s [fd:%d, addr:%.80s, len:%d]",
             strerror(errno),
             fd,
             httpd_ntoa(&sa),
             sz);

      if (errno == EINVAL) {
        perror("accept");
        exit(255);
      }
      return GC_FAIL;
    }
  }

  if (!sockaddr_check(&sa)) {
    syslog(LOG_ERR, "Unknown sockaddr family");
    close(conn->conn_fd);
    return GC_FAIL;
  }

  fcntl(conn->conn_fd, F_SETFD, 1);
  conn->server         = hs;
  conn->read_buf       = NULL;
  conn->read_size      = 0;
  conn->read_idx       = 0;
  conn->checked_idx    = 0;
  conn->bytes_to_send  = 0;
  conn->bytes_sent     = 0;
  conn->method         = HTTP_METHOD_UNKNOWN;
  conn->status         = 0;
  conn->should_linger  = 0;
  conn->encoded_url    = "";
  conn->decoded_url[0] = '\0';
  conn->protocol       = "UNKNOWN";
  conn->reqhost[0]     = '\0';
  conn->query[0]       = '\0';
  conn->path[0]        = '\0';
  conn->hdrhost        = "";
  conn->mime_flag      = 1;
  conn->data_address   = NULL;

  memset(&conn->client_addr, 0, sizeof(conn->client_addr));
  memmove(&conn->client_addr, &sa, sockaddr_len(&sa));
  
  return GC_OK;
}

void
httpd_close_conn(http_conn_t *conn)
{
  if (conn->conn_fd >= 0) {
    close(conn->conn_fd);
    conn->conn_fd = -1;
  }
}

size_t
httpd_write_fully(int fd, const char *buf, size_t sz)
{
  size_t written = 0;

  while (written < sz) {
    int r = 0;

    r = write(fd, buf + written, sz - written);
    if (r < 0 && (errno == EINTR || errno == EAGAIN)) {
      sleep(1);
      continue;
    }
    
    if (r < 0) {
      return r;
    }

    if (r == 0) {
      break;
    }

    written += r;
  }

  return written;
}

void
httpd_write_response(http_conn_t *conn)
{
  if (conn->response_len > 0) {
    httpd_write_fully(conn->conn_fd, conn->response, conn->response_len);
    conn->response_len = 0;
  }
}

void
httpd_send_err(http_conn_t *conn,
              int          status,
              char        *title,
              char        *extraheads,
              char        *form)
{
  send_response(conn, status, title, extraheads, form);
}

int
httpd_got_request(http_conn_t *conn)
{
  char ch = 0;

  for (; conn->checked_idx < conn->read_idx; ++conn->checked_idx) {
    ch = conn->read_buf[conn->checked_idx];

#define WHITESPACE       case ' ':    case '\t'
#define CRLF             case '\012': case '\015'

    switch (conn->checked_state) {
      case CHST_FIRSTWORD:
        switch (ch) {
          WHITESPACE:
            conn->checked_state = CHST_FIRSTWS;
            break;
          CRLF:
            conn->checked_state = CHST_BOGUS;
            return GR_BAD_REQUEST;
        }
        break;

      case CHST_FIRSTWS:
        switch (ch) {
          WHITESPACE:
            break;
          CRLF:
            conn->checked_state = CHST_BOGUS;
            return GR_BAD_REQUEST;
          default:
            conn->checked_state = CHST_SECONDWORD;
            break;
        }
        break;

      case CHST_SECONDWORD:
        switch (ch) {
          WHITESPACE:
            conn->checked_state = CHST_SECONDWS;
            break;
          CRLF:
            return GR_GOT_REQUEST;
        }
        break;

      case CHST_SECONDWS:
        switch (ch) {
          WHITESPACE:
            break;
          CRLF:
            conn->checked_state = CHST_BOGUS;
            return GR_BAD_REQUEST;
          default:
            conn->checked_state = CHST_THIRDWORD;
            break;
        }
        break;

      case CHST_THIRDWORD:
        switch (ch) {
          WHITESPACE:
            conn->checked_state = CHST_THIRDWS;
            break;
          case '\012':
            conn->checked_state = CHST_LF;
            break;
          case '\015':
            conn->checked_state = CHST_CR;
            break;
        }
        break;

      case CHST_THIRDWS:
        switch (ch) {
          WHITESPACE:
            break;
          case '\012':
            conn->checked_state = CHST_LF;
            break;
          case '\015':
            conn->checked_state = CHST_CR;
            break;
          default:
            conn->checked_state = CHST_BOGUS;
            return GR_BAD_REQUEST;
        }
        break;

      case CHST_LINE:
        switch (ch) {
          case '\012':
            conn->checked_state = CHST_LF;
            break;
          case '\015':
            conn->checked_state = CHST_CR;
            break;
        }
        break;

      case CHST_LF:
        switch (ch) {
          case '\012':
            return GR_GOT_REQUEST;
          case '\015':
            conn->checked_state = CHST_CR;
            break;
          default:
            conn->checked_state = CHST_LINE;
            break;
        }
        break;

      case CHST_CR:
        switch (ch) {
          case '\012':
            conn->checked_state = CHST_CRLF;
            break;
          case '\015':
            return GR_GOT_REQUEST;
          default:
            conn->checked_state = CHST_LINE;
            break;
        }
        break;

      case CHST_CRLF:
        switch (ch) {
          case '\012':
            return GR_GOT_REQUEST;
          case '\015':
            conn->checked_state = CHST_CRLFCR;
            break;
          default:
            conn->checked_state = CHST_LINE;
            break;
        }
        break;

      case CHST_CRLFCR:
        switch (ch) {
          CRLF:
            return GR_GOT_REQUEST;
          default:
            conn->checked_state = CHST_LINE;
            break;
        }
        break;

      case CHST_BOGUS:
        return GR_BAD_REQUEST;
    }

#undef CRLF
#undef WHITESPACE
  }

  return GR_NO_REQUEST;
}

char *
httpd_method_str(int method)
{
  static char *method_get  = "GET";
  static char *method_head = "HEAD";
  static char *method_post = "POST";
  static char *unknown     = "UNKNOWN";

  switch (method) {
    case HTTP_METHOD_GET:  return method_get;
    case HTTP_METHOD_HEAD: return method_head;
    case HTTP_METHOD_POST: return method_post;
    default:               return unknown;
  }
}

int
httpd_parse_request(http_conn_t *conn)
{
  char *buf        = NULL;
  char *method_str = NULL;
  char *url        = NULL;
  char *protocol   = NULL;
  char *reqhost    = NULL;
  char *eol        = NULL;
  char *cp         = NULL;

#define WHITESPACE " \t\012\015"

  conn->checked_idx = 0;
  method_str        = bufgets(conn);

  url = strpbrk(method_str, WHITESPACE);
  if (url == NULL) {
    printf("URL is NULL...\n");
    httpd_send_err(conn, 400, err400title, "", err400form);
    return -1;
  }

  *url++ = '\0';
  url += strspn(url, WHITESPACE);

  protocol = strpbrk(url, WHITESPACE);
  if (protocol == NULL) {
    protocol        = (char *)proto09;
    conn->mime_flag = 0;
  } else {
    *protocol++ = '\0';
    protocol += strspn(protocol, WHITESPACE);

    if (*protocol != '\0') {
      eol = strpbrk(protocol, WHITESPACE);

      if (eol != NULL) {
        *eol = '\0';
      }

      if (strcasecmp(protocol, proto10) != 0) {
        conn->one_one = 1;
      }
    }
  }
  conn->protocol = protocol;

#undef WHITESPACE

  printf("protocol is [%s]\n", protocol);
  printf("url is [%s], does it have http://?\n", url);

  if (strncasecmp(url, "http://", 7) == 0) {
    if (!conn->one_one) {
      printf("HTTP/1.1 and you're trying HTTP/1.0 or 0.9\n");
      httpd_send_err(conn, 400, err400title, "", err400form);
      return -1;
    }

    reqhost = url + 7;
    url = strchr(reqhost, '/');
    if (url == NULL) {
      printf("URL is NULL after host.\n");
      httpd_send_err(conn, 400, err400title, "", err400form);
      return -1;
    }
    *url = '\0';

    if (strchr(reqhost, '/') != NULL || reqhost[0] == '.') {
      printf("reqhost is derp.\n");
      httpd_send_err(conn, 400, err400title, "", err400form);
      return -1;
    }

    httpd_realloc_str(&conn->reqhost, &conn->max_reqhost, strlen(reqhost));
    strcpy(conn->reqhost, reqhost);
    *url = '/';
  }

  if (*url != '/') {
    printf("URL != /\n");
    httpd_send_err(conn, 400, err400title, "", err400form);
    return -1;
  }

  if (strcasecmp(method_str, httpd_method_str(HTTP_METHOD_GET)) == 0) {
    conn->method = HTTP_METHOD_GET;
  } else if (strcasecmp(method_str, httpd_method_str(HTTP_METHOD_HEAD)) == 0) {
    conn->method = HTTP_METHOD_HEAD;
  } else if (strcasecmp(method_str, httpd_method_str(HTTP_METHOD_POST)) == 0) {
    conn->method = HTTP_METHOD_POST;
  } else {
    httpd_send_err(conn, 501, err501title, "", err501form);
    return -1;
  }

  conn->encoded_url = url;
  httpd_realloc_str(&conn->decoded_url,
                    &conn->max_decoded_url,
                    strlen(conn->encoded_url));
  strdecode(conn->decoded_url, conn->encoded_url);

  httpd_realloc_str(&conn->path,
                    &conn->max_path,
                    strlen(conn->decoded_url));
  strcpy(conn->path, &conn->decoded_url[1]);
  if (conn->path[0] == '\0') {
    strcpy(conn->path, ".");
  }

  cp = strchr(conn->encoded_url, '?');
  if (cp != NULL) {
    ++cp;
    httpd_realloc_str(&conn->query,
                      &conn->max_query,
                      strlen(cp));
    strcpy(conn->query, cp);

    cp = strchr(conn->path, '?');
    if (cp != NULL) {
      *cp = '\0';
    }
  }

  de_dotdot(conn->path);
  if (conn->path[0]  == '/' ||
      (conn->path[0] == '.' && conn->path[1] == '.' &&
       (conn->path[2] == '\0' || conn->path[2] == '/')))
  {
    printf("Path is broken.\n");
    httpd_send_err(conn, 400, err400title, "", err400form);
    return -1;
  }

  if (conn->mime_flag) {
    while ((buf = bufgets(conn)) != NULL) {
      if (buf[0] == '\0') {
        break;
      }

      if (strncasecmp(buf, "Host:", 5) == 0) {
        printf("Got a Host header\n");

        cp             = &buf[5];
        cp            += strspn(cp, " \t");
        conn->hdrhost  = cp;

        cp = strchr(conn->hdrhost, ':');
        if (cp != NULL) {
          *cp = '\0';
        }

        if (strchr(conn->hdrhost, '/') != NULL ||
            conn->hdrhost [0]          == '.')
        {
          printf("No Host: header.\n");
          httpd_send_err(conn, 400, err400title, "", err400form);
          return -1;
        }
      }
    }
  }

  if (conn->one_one) {
    if (conn->reqhost[0] == '\0' && conn->hdrhost[0] == '\0') {
      printf("Stupid Host header [%s]\n", conn->reqhost);
      httpd_send_err(conn, 400, err400title, "", err400form);
      return -1;
    }
  }

  return 0;
}

static
int
really_start_request(http_conn_t *conn, struct timeval *tv)
{
  endpoint_t  *node = NULL;
  json_node_t *obj  = NULL;
  sm_all_t    *inst = NULL;

  if (conn->method != HTTP_METHOD_GET  &&
      conn->method != HTTP_METHOD_HEAD &&
      conn->method != HTTP_METHOD_POST)
  {
    httpd_send_err(conn, 501, err501title, "", err501form);
    return -1;
  }

  printf("Request\n");
  printf("  Method:   %s\n", httpd_method_str(conn->method));
  printf("  Protocol: %s\n", conn->protocol);
  printf("  URL:      %s\n", conn->decoded_url);
  printf("  Query:    %s\n", conn->query);
  printf("  Path:     %s\n", conn->path);

  node = endpoint_find(conn->path);
  if (node == NULL) {
    httpd_send_err(conn, 404, err404title, "", err404form);
    return -1;
  }

  inst = (sm_all_t *)node->instance;
  obj  = json_mkobject();
  (inst->vtab->emit_json)(&obj);
  conn->data_address = json_stringify(obj, NULL);
  //conn->bytes_to_send = strlen(conn->data_address);

  if (conn->data_address == NULL) {
    httpd_send_err(conn, 500, err500title, "", err500form);
    return -1;
  }

  send_mime(conn,
            200,
            ok200title,
            "",
            "",
            "application/json",
            strlen(conn->data_address),
            tv->tv_sec);
  return 0;
}

int
httpd_start_request(http_conn_t *conn, struct timeval *tv)
{
  return really_start_request(conn, tv);
}

/* httpd.c ends here. */
