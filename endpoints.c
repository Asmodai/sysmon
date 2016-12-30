/*
 * endpoint.c --- Endpoints implementation.
 *
 * Copyright (c) 2016 Paul Ward <asmodai@gmail.com>
 *
 * Author:     Paul Ward <asmodai@gmail.com>
 * Maintainer: Paul Ward <asmodai@gmail.com>
 * Created:    29 Dec 2016 23:24:18
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
 * @file endpoint.c
 * @author Paul Ward
 * @brief Endpoints implementation.
 */

#include <sys/param.h>
#include <sys/types.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>

#include "utils.h"
#include "endpoints.h"

#ifdef BSD
extern char *strdup();
#endif

endpoint_t *endpoints = NULL;

void
endpoint_init(void)
{
  endpoints = NULL;
}

endpoint_t *
endpoint_create(const char *name, const void *instance)
{
  endpoint_t *node = xmalloc(sizeof(endpoint_t));

  node->hash     = pjw_hash(name);
  node->name     = strdup(name);
  node->instance = instance;
  node->next     = NULL;
  node->prev     = NULL;

  if (endpoints == NULL) {
    endpoints = node;
    goto out;
  }

  endpoints->prev = node;
  node->next      = endpoints;
  endpoints       = node;

out:
  return node;
}

endpoint_t *
endpoint_find(const char *name)
{
  unsigned long  hash = pjw_hash(name);
  endpoint_t *node = endpoints;

  while (node != NULL) {
    if (node->hash == hash) {
      return node;
    }

    node = node->next;
  }

  return NULL;
}

void
endpoint_traverse(void (*callback)(const void *))
{
  endpoint_t *node = endpoints;

  for (; node != NULL; node = node->next) {
    (callback)(node->instance);
  }
}

/* endpoint.c ends here. */
