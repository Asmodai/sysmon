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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "httpd.h"
#include "utils.h"
#include "endpoints.h"
#include "json.h"
#include "sm_all.h"

#define BUFSIZE   8096

#ifndef SIGCLD
# define SIGCLD SIGCHLD
#endif

static char notfoundmsg[] = 
    "HTTP/1.0 404 Not Found\n"
    "Content-Length: 15\n"
    "Connection: close\n"
    "Content-Type: text/plain\n\n"
    "404 Not Found.\n";

static char forbiddenmsg[] = 
    "HTTP/1.0 403 Forbidden\n"
    "Content-Length: 14\n"
    "Connection: close\n"
    "Content-Type: text/plain\n\n"
    "403 Forbidden\n";

static char internalmsg[] = 
    "HTTP/1.0 500 Internal Server Error\n"
    "Content-Length: 26\n"
    "Connection: close\n"
    "Content-Type: text/plain\n\n"
    "500 Internal Server Error\n";

void
http_logger(int type, char *title, char *message, int sockfd)
{
  int fd                      = 0;
  char logbuffer[BUFSIZE * 2] = { 0 };

  switch (type) {
    case HTTP_LOG:
      sprintf(logbuffer, "INFO: %s:%s:%d", title, message, sockfd);
      break;

    case HTTP_ERROR:
      sprintf(logbuffer, "FATAL ERROR: %s:%s Errno = %d",
              title,
              message,
              errno);
      break;

    case HTTP_FORBIDDEN:
      write(sockfd, forbiddenmsg, strlen(forbiddenmsg));
      sprintf(logbuffer, "FORBIDDEN: %s:%s", title, message);
      break;

    case HTTP_NOTFOUND:
      write(sockfd, notfoundmsg, strlen(notfoundmsg));
      sprintf(logbuffer, "NOT FOUND: %s:%s", title, message);
      break;

    case HTTP_INTERNAL:
      write(sockfd, internalmsg, strlen(internalmsg));
      sprintf(logbuffer, "INTERNAL SERVER ERROR: %s:%s", title, message);
      break;
  }

  // SYSLOG LOGGER HERE.

  if ((fd = open("http.log", O_CREAT | O_WRONLY | O_APPEND,  0644)) >= 0) {
    write(fd, logbuffer, strlen(logbuffer));
    write(fd, "\n", 1);
    close(fd);
  }

  if (type != HTTP_LOG) {
    exit(3);
  }
}

void
http_server(int fd, int hit)
{
  long         ret                = 0;
  size_t       i                  = 0;
  size_t       j                  = 0;
  endpoint_t  *node               = NULL;
  json_node_t *obj                = NULL;
  sm_all_t    *inst               = NULL;
  char        *json               = NULL;
  static char buffer[BUFSIZE + 1] = { 0 };

  ret = read(fd, buffer, BUFSIZE);

  if (ret == 0 || ret == -1) {
    http_logger(HTTP_INTERNAL, "Failed to read browser request", "", fd);
  }

  if (ret > 0 && ret < BUFSIZE) {
    buffer[ret] = 0;
  } else {
    buffer[0] = 0;
  }

  for (i = 0; i < (size_t)ret; i++) {
    if (buffer[i] == '\r' || buffer[i] == '\n') {
      buffer[i] = '*';
    }
  }

  if (strncmp(buffer, "GET ", 4)) {
    http_logger(HTTP_FORBIDDEN, "Only GET supported", buffer, fd);
  }

  for (i = 4; i < BUFSIZE; i++) {
    if (buffer[i] == ' ') {
      buffer[i] = 0;
      break;
    }
  }

  for (j = 0; i < i - 1; j++) {
    if (buffer[j] == '.' && buffer[j + 1] == '.') {
      http_logger(HTTP_FORBIDDEN, "Path name .. not supported.", buffer, fd);
    }
  }

  node = endpoint_find(&buffer[5]);
  if (node == NULL) {
    http_logger(HTTP_NOTFOUND, "Nope.", &buffer[5], fd);
  }

  inst = (sm_all_t *)node->instance;
  obj  = json_mkobject();

  (inst->vtab->emit_json)(&obj);
  json = json_stringify(obj, NULL);

  sprintf(buffer,
          "HTTP/1.0 200 OK\n"
          "Content-Length: %ld\n"
          "Connection: close\n"
          "Content-Type: text/plain\n\n",
          strlen(json));
  write(fd, buffer, strlen(buffer));
  write(fd, json, strlen(json));
  write(fd, "\n", 1);
  free(json);
  json_delete(obj);

  sleep(1);
  close(fd);
  exit(1);
}

void
http_spawn(void)
{
  int                       listenfd = 0;
  int                       sockfd   = 0;
  int                       hit      = 0;
  socklen_t                 length;
  pid_t                     pid      = 0;
  static struct sockaddr_in cli_addr;
  static struct sockaddr_in srv_addr;
  /*
  if (fork() != 0) {
    return;
  }
  */

  signal(SIGCLD, SIG_IGN);
  signal(SIGHUP, SIG_IGN);

  setpgrp();
  fprintf(stdout, "starting web server.\n");

  if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    http_logger(HTTP_ERROR, "System call", "socket", 0);
  }

  srv_addr.sin_family      = AF_INET;
  srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  srv_addr.sin_port        = htons(HTTP_PORT);

  if (bind(listenfd, (struct sockaddr *)&srv_addr, sizeof(srv_addr)) < 0) {
    http_logger(HTTP_ERROR, "system call", "bind", 0);
  }

  if (listen(listenfd, 64) < 0) {
    http_logger(HTTP_ERROR, "system call", "listen", 0);
  }

  for (hit = 1; ; hit++) {
    length = sizeof(cli_addr);

    if ((sockfd = accept(listenfd, (struct sockaddr *)&cli_addr, &length)) < 0)
    {
      http_logger(HTTP_ERROR, "system call", "accept", 0);
    }

    if ((pid = fork()) < 0) {
      http_logger(HTTP_ERROR, "system call", "fork", 0);
    } else {
      if (pid == 0) {
        close(listenfd);
        http_server(sockfd, hit);
      } else {
        close(sockfd);
      }
    }
  }
}

/* httpd.c ends here. */
