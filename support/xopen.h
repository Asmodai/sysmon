/*
 * xopen.h --- X/Open standard detection.
 *
 * Copyright (c) 2017 Paul Ward <asmodai@gmail.com>
 *
 * Author:     Paul Ward <asmodai@gmail.com>
 * Maintainer: Paul Ward <asmodai@gmail.com>
 * Created:    10 Mar 2017 17:21:43
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
 * @file xopen.h
 * @author Paul Ward
 * @brief X/Open standard detection.
 */

#ifndef _xopen_h_
#define _xopen_h_

#include <unistd.h>

/*
 * X/Open standards.
 */
#define XOPEN_NONE    0x00              /* No X/Open standard.         */
#define XOPEN_XPG3    0x01              /* X/Open Portability Guide 3. */
#define XOPEN_XPG4    0x02              /* X/Open Portability Guide 4. */
#define XOPEN_UNIX95  0x04              /* X/Open Single UNIX Spec.    */
#define XOPEN_UNIX98  0x08              /* X/Open Single UNIX Spec v2. */
#define XOPEN_UNIX03  0x10              /* X/Open Single UNIX Spec v3. */
#define XOPEN_SUS4    0x20              /* X/Open Single UNIX Spec v4. */

#if defined(_XOPEN_VERSION)
# if (_XOPEN_VERSION == 3)
#  define XOPEN_STANDARD    XOPEN_XPG3
# elif (_XOPEN_VERSION == 4)
#  if defined(_XOPEN_UNIX)
#   define XOPEN_STANDARD   XOPEN_UNIX95
#  else
#   define XOPEN_STANDARD   XOPEN_XPG4
#  endif
# elif (_XOPEN_VERSION == 500)
#  define XOPEN_STANDARD    XOPEN_UNIX98
# elif (_XOPEN_VERSION == 600)
#  define XOPEN_STANDARD    XOPEN_UNIX03
# elif (_XOPEN_VERSION == 700)
#  define XOPEN_STANDARD    XOPEN_SUS4
# endif
#else
# define XOPEN_STANDARD     XOPEN_NONE
#endif

#define XOPEN_EQ(__s) \
  (XOPEN_STANDARD == __s)

#define XOPEN_GT(__s) \
  (XOPEN_STANDARD >= __s)

#define XOPEN_LT(__s) \
  (XOPEN_STANDARD < __s)

#endif /* !_xopen_h_ */

/* xopen.h ends here. */
