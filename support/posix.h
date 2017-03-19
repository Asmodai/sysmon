/*
 * posix.h --- POSIX detection.
 *
 * Copyright (c) 2017 Paul Ward <asmodai@gmail.com>
 *
 * Author:     Paul Ward <asmodai@gmail.com>
 * Maintainer: Paul Ward <asmodai@gmail.com>
 * Created:    10 Mar 2017 16:59:00
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
 * @file posix.h
 * @author Paul Ward
 * @brief POSIX detection.
 */

#ifndef _posix_h_
#define _posix_h_

#include <unistd.h>

/*
 * POSIX standards.
 */
#define POSIX_NONE     0x00             /* No POSIX standard.   */
#define POSIX_1_1988   0x01             /* POSIX.1-1988.        */
#define POSIX_1_1990   0x02             /* ISO/IEC 9945-1:1990. */
#define POSIX_2        0x04             /* ISO/IEC 9945-2:1993. */
#define POSIX_1B_1993  0x08             /* IEEE 1003.1b-1993.   */
#define POSIX_1_1996   0x10             /* IEEE 1003.1-1996.    */
#define POSIX_1_2001   0x20             /* IEEE 1003.1-2001.    */
#define POSIX_1_2008   0x40             /* IEEE 1003.1-2008.    */

/*
 * Attempt to detect POSIX standard in use.
 */
#if defined(_POSIX_VERSION)
# if (_POSIX_VERSION == 198808L)
#  define POSIX_STANDARD    POSIX_1_1988
# elif (_POSIX_VERSION == 19909L)
#  define POSIX_STANDARD    POSIX_1_1990
# elif (_POSIX_VERSION == 199309L)
#  define POSIX_STANDARD    POSIX_1B_1993
# elif (_POSIX_VERSION == 199506L)
#  define POSIX_STANDARD    POSIX_1_1996
# elif (_POSIX_VERSION == 200112L)
#  define POSIX_STANDARD    POSIX_1_2001
# elif (_POSIX_VERSION == 200809L)
#  define POSIX_STANDARD    POSIX_1_2008
# endif
#elif defined(_POSIX2_C_VERSION)
# if (_POSIX2_C_VERSION == 199209L)
#  define POSIX_STANDARD    POSIX_2
# else
#  define POSIX_STANDARD    POSIX_NONE
# endif
#else
# define POSIX_STANDARD     POSIX_NONE
#endif

#define POSIX_EQ(__s) \
  (POSIX_STANDARD == __s)

#define POSIX_GT(__s) \
  (POSIX_STANDARD >= __s)

#define POSIX_LT(__s) \
  (POSIX_STANDARD < __s)

#endif /* !_posix_h_ */

/* posix.h ends here. */
