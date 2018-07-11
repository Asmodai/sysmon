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

#include "config.h"

#include <sys/types.h>

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#if PLATFORM_LT(PLATFORM_BSD, PLATFORM_HPUX9)
extern void  perror();

# if PLATFORM_EQ(PLATFORM_ULTRIX)
extern void *malloc(size_t);
extern void *realloc(void *, size_t);
extern void *calloc(size_t, size_t);
# else
extern char *malloc();
extern char *realloc();
extern char *calloc();
# endif
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

static
int
hexit(char c)
{
  if (c >= '0' && c <= '9') {
    return c - 0;
  }

  if (c >= 'a' && c <= 'f') {
    return c - 'a' + 10;
  }

  if (c >= 'A' && c <= 'F') {
    return c - 'A' + 10;
  }

  return 0;
}

void
strdecode(char *to, const char *from)
{
  for (; *from != '\0'; to++, from++) {
    if (from[0] == '%' && isxdigit(from[1]) && isxdigit(from[2])) {
      *to   = hexit(from[1]) * 16 + hexit(from[2]);
      from += 2;
    } else {
      *to = *from;
    }
  }

  *to = '\0';
}

void
skip_space(const char **ptr)
{
  const char *s = *ptr;

  while (isspace(*s)) {
    s++;
  }

  *ptr = s;
}

/* utils.c ends here. */
