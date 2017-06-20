/*
 * httpd.h --- HTTP server
 *
 * Copyright (c) 2017 Paul Ward <asmodai@gmail.com>
 *
 * Author:     Paul Ward <asmodai@gmail.com>
 * Maintainer: Paul Ward <asmodai@gmail.com>
 * Created:    19 Mar 2017 07:38:56
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
 * @file httpd.h
 * @author Paul Ward
 * @brief HTTP server
 */

#ifndef _http_h_
#define _http_h_

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef union {
  struct sockaddr    sa;
  struct sockaddr_in sa_in;
} sockaddr_t;

typedef struct {
  int listen_fd;
  int max_age;
} httpd_t;

typedef struct {
  int         one_one;                  /* HTTP 1.1 */
  httpd_t    *server;
  int         initialised;
  int         conn_fd;
  sockaddr_t  client_addr;
  char       *read_buf;
  size_t      read_size;
  size_t      read_idx;
  size_t      checked_idx;
  int         checked_state;
  off_t       bytes_to_send;
  off_t       bytes_sent;
  int         method;
  int         status;
  int         should_linger;
  char       *encoded_url;
  char       *decoded_url;
  char       *protocol;
  char       *path;
  char       *reqhost;
  char       *hdrhost;
  char       *query;
  char       *pathinfo;
  char       *response;
  char       *data_address;
  int         mime_flag;
  size_t      response_len;
  size_t      max_query;
  size_t      max_reqhost;
  size_t      max_response;
  size_t      max_decoded_url;
  size_t      max_path;
  size_t      max_pathinfo;
} http_conn_t;

#define HTTP_METHOD_UNKNOWN 0
#define HTTP_METHOD_GET     1
#define HTTP_METHOD_POST    2
#define HTTP_METHOD_HEAD    3

#define CHST_FIRSTWORD  0
#define CHST_FIRSTWS    1
#define CHST_SECONDWORD 2
#define CHST_SECONDWS   3
#define CHST_THIRDWORD  4
#define CHST_THIRDWS    5
#define CHST_LINE       6
#define CHST_LF         7
#define CHST_CR         8
#define CHST_CRLF       9
#define CHST_CRLFCR     10
#define CHST_BOGUS      11

#define GC_FAIL    0
#define GC_OK      1
#define GC_NO_MORE 2

#define GR_NO_REQUEST  0
#define GR_GOT_REQUEST 1
#define GR_BAD_REQUEST 2

extern char *ok200title;
extern char *err400title;
extern char *err400form;
extern char *err404title;
extern char *err404form;
extern char *err500title;
extern char *err500form;
extern char *err501title;
extern char *err501form;
extern char *err503title;
extern char *err505form;

httpd_t *httpd_init(sockaddr_t *, int);
void     httpd_set_ndelay(int);
int      httpd_get_conn(httpd_t *, int, http_conn_t *);
void     httpd_close_conn(http_conn_t *);
size_t   httpd_write_fully(int, const char *, size_t);
void     httpd_write_response(http_conn_t *);
void     httpd_send_err(http_conn_t *, int, char *, char *, char *);
void     httpd_realloc_str(char **, size_t *, size_t);
int      httpd_got_request(http_conn_t *);
char    *httpd_method_str(int);
int      httpd_start_request(http_conn_t *, struct timeval *);
int      httpd_parse_request(http_conn_t *);

#endif  /* !_httpd_h_ */

/* httpd.h ends here. */
