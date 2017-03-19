/*
 * support.h --- Support stuff.
 *
 * Copyright (c) 2016 Paul Ward <asmodai@gmail.com>
 *
 * Author:     Paul Ward <asmodai@gmail.com>
 * Maintainer: Paul Ward <asmodai@gmail.com>
 * Created:    31 Dec 2016 20:20:25
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
 * @file support.h
 * @author Paul Ward
 * @brief Support stuff.
 */

#ifndef _support_h_
#define _support_h_

#include "platform.h"

#if defined(__WORDSIZE)
# if __WORDSIZE == 64
#  define BITS_64
# else
#  define BITS_32
# endif
#else
# define BITS_32
# define __WORDSIZE  32
#endif

#if PLATFORM_EQ(PLATFORM_BSD)
# if PLATFORM_EQ(PLATFORM_42BSD) || PLATFORM_EQ(PLATFORM_43BSD) || \
  PLATFORM_EQ(PLATFORM_44BSD)
#  include <machine/endian.h>
# elif PLATFORM_EQ(PLATFORM_ULTRIX)
#  define BIG_ENDIAN    3412
#  define LITTLE_ENDIAN 1234
#  if ARCHITECTURE_EQ(ARCHITECTURE_VAX)
#   define BYTE_ORDER LITTLE_ENDIAN
#  elif ARCHITECTURE_EQ(ARCHITECTURE_MIPS)
/* TODO: fixme. */
#   define BYTE_ORDER LITTLE_ENDIAN
#  endif
# endif
#elif PLATFORM_EQ(PLATFORM_UNIX)
# include <endian.h>
#else
# define BIG_ENDIAN    3412
# define LITTLE_ENDIAN 1234
# define BYTE_ORDER    LITTLE_ENDIAN
#endif

#endif /* !_support_h_ */

/* support.h ends here. */
