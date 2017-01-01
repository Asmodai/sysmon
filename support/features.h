/*
 * features.h --- Platform features.
 *
 * Copyright (c) 2017 Paul Ward <asmodai@gmail.com>
 *
 * Author:     Paul Ward <asmodai@gmail.com>
 * Maintainer: Paul Ward <asmodai@gmail.com>
 * Created:    01 Jan 2017 08:15:16
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
 * @file features.h
 * @author Paul Ward
 * @brief Platform features.
 */

#ifndef _features_h_
#define _features_h_

#include "platform.h"

/*
 * The following systems do not have a `uname` syscall:
 *
 *   o 4.2 BSD,
 *   o 4.3 BSD,
 *   o NeXTSTEP,
 *   o OPENSTEP,
 *   o Rhapsody
 */
#if PLATFORM_LT(PLATFORM_BSD, PLATFORM_44BSD) || \
  PLATFORM_LT(PLATFORM_NEXT, PLATFORM_MACOSX)
# undef HAVE_SYS_UTSNAME_H
#else
# define HAVE_SYS_UTSNAME_H
#endif

/*
 * Lots of older systems lack stdint.h and stdbool.h
 */
#if PLATFORM_EQ(PLATFORM_LINUX) || \
  PLATFORM_GTE(PLATFORM_BSD, PLATFORM_FREEBSD) || \
  PLATFORM_GTE(PLATFORM_NEXT, PLATFORM_OSX) || \
  PLATFORM_GTE(PLATFORM_SVR4, PLATFORM_SOLARIS) || \
  PLATFORM_GTE(PLATFORM_SVR3, PLATFORM_AIX)
# define HAVE_STDINT_H
# define HAVE_STDBOOL_H
#endif

#endif /* !_features_h_ */

/* features.h ends here. */
