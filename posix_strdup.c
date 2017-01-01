/*
 * posix_strdup.c --- `strdup' for platforms that lack it.
 *
 * Copyright (c) 2016 Paul Ward <asmodai@gmail.com>
 *
 * Author:     Paul Ward <asmodai@gmail.com>
 * Maintainer: Paul Ward <asmodai@gmail.com>
 * Created:    30 Dec 2016 05:15:54
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
 * @file posix_strdup.c
 * @author Paul Ward
 * @brief `strdup' for platforms that lack it.
 */

#include "config.h"

#include <sys/types.h>

#include <stdlib.h>
#include <string.h>

#if PLATFORM_EQ(PLATFORM_BSD)
# if PLATFORM_LT(PLATFORM_BSD, PLATFORM_BSDOS)
#  if PLATFORM_GTE(PLATFORM_BSD, PLATFORM_ULTRIX)
extern void *malloc(size_t);
#  else
extern char *malloc();
extern char *memcpy();
#  endif  /* BSD > ULTRIX */
# endif  /* BSD < BSDOS */
#endif

char *
strdup(const char *s)
{
  size_t  len = strlen(s) + 1;
  void   *new = malloc(len);

  if (new == NULL) {
    return NULL;
  }

  return memcpy(new, s, len);
}

/* posix_strdup.c ends here. */
