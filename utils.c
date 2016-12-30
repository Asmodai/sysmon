/*
 * utils.c --- Various utilities.
 *
 * Copyright (c) 2016 Paul Ward <asmodai@gmail.com>
 *
 * Author:     Paul Ward <asmodai@gmail.com>
 * Maintainer: Paul Ward <asmodai@gmail.com>
 * Created:    30 Dec 2016 00:59:14
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
 * @file utils.c
 * @author Paul Ward
 * @brief Various utilities.
 */

#include <sys/param.h>
#include <sys/types.h>

#include <stdlib.h>
#include <stdio.h>

#ifdef BSD
extern       perror();
extern char *malloc();
#endif

void *
xmalloc(size_t n)
{
  void *p = NULL;

  if ((p = malloc(n)) == 0) {
    perror("xmalloc");
    exit(1);
  }

  return p;
}

void *
xrealloc(void *ptr, size_t n)
{
  if ((ptr = realloc(ptr, n)) == 0) {
    perror("xrealloc");
    exit(1);
  }

  return ptr;
}

void *
xcalloc(size_t nelem, size_t elsize)
{
  void *newmem = calloc(nelem ? nelem : 1, elsize ? elsize : 1);

  if (newmem == NULL) {
    perror("xcalloc");
    exit(1);
  }

  return newmem;
}

unsigned long
pjw_hash(const char *s)
{
  unsigned long h    = 0;
  unsigned long high = 0;

  while (*s) {
    h = (h << 4) + *s++;

    if ((high = h & 0xF0000000)) {
      h ^= high >> 24;
    }

    h &= ~high;
  }

  return h;
}

/* utils.c ends here. */
