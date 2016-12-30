/*
 * endpoint.h --- REST API endpoints.
 *
 * Copyright (c) 2016 Paul Ward <asmodai@gmail.com>
 *
 * Author:     Paul Ward <asmodai@gmail.com>
 * Maintainer: Paul Ward <asmodai@gmail.com>
 * Created:    29 Dec 2016 23:19:27
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
 * @file endpoint.h
 * @author Paul Ward
 * @brief REST API endpoints.
 */

#ifndef _endpoint_h_
#define _endpoint_h_

#include <sys/types.h>

typedef struct endpoint_s {
  struct endpoint_s *prev;              /* Next in list. */
  struct endpoint_s *next;              /* Previous in list. */
  unsigned long      hash;              /* Hash of name for lookup. */
  const char        *name;              /* Name string. */
  const void        *instance;          /* Instance of handler. */
} endpoint_t;

void        endpoint_init(void);
endpoint_t *endpoint_create(const char *name, const void *instance);
endpoint_t *endpoint_find(const char *name);
void        endpoint_traverse(void (*callback)(const void *));

#define endpoint_foreach(i) \
  for ((i) = endpoints; (i) != NULL; (i) = (i)->next)

#endif /* !_endpoint_h_ */

/* endpoint.h ends here. */
