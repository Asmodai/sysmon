/*
 * platform.h --- Platform detection.
 *
 * Copyright (c) 2016 Paul Ward <asmodai@gmail.com>
 *
 * Author:     Paul Ward <asmodai@gmail.com>
 * Maintainer: Paul Ward <asmodai@gmail.com>
 * Created:    31 Dec 2016 20:06:26
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
 * For now this list just includes platforms that I use.  That is to say, the
 * list is incomplete.
 */
/* }}} */

/**
 * @file platform.h
 * @author Paul Ward
 * @brief Platform detection.
 */

#ifndef _platform_h_
#define _platform_h_

/* We need this for older BSDs. */
#include <sys/param.h>

#define PLATFORM_UNIX      0x10000000   /* Unix or Unix-like family.       */
#define PLATFORM_LINUX     0x10000001   /* |-- GNU/Linux.                  */
#define PLATFORM_BSD       0x11000000   /* |-- BSD family.                 */
#define PLATFORM_42BSD     0x11000001   /* | |-- 4.2 BSD.                  */
#define PLATFORM_43BSD     0x11000002   /* | |-- 4.3 BSD.                  */
#define PLATFORM_44BSD     0x11000004   /* | |-- 4.4 BSD.                  */
#define PLATFORM_ULTRIX    0x11000008   /* | |-- DEC Ultrix.               */
#define PLATFORM_SUNOS     0x11000010   /* | |-- Sun SunOS.                */
#define PLATFORM_HPUX9     0x11000020   /* | |-- HP HP-UX <= 9.x.          */
#define PLATFORM_BSDOS     0x11000040   /* | |-- BSDi BSD/OS.              */
#define PLATFORM_FREEBSD   0x11000080   /* | |-- FreeBSD.                  */
#define PLATFORM_NETBSD    0x11000100   /* | |-- NetBSD.                   */
#define PLATFORM_OPENBSD   0x11000200   /* | |-- OpenBSD.                  */
#define PLATFORM_DRAGONFLY 0x11000400   /* | |-- DragonFly BSD.            */
#define PLATFORM_MACH      0x11100000   /* | `-- CMU Mach family.          */
#define PLATFORM_OSF1      0x01100001   /* |   |-- DEC OSF1/Tru64.         */
#define PLATFORM_NEXT      0x11110000   /* |   `-- NeXT family.            */
#define PLATFORM_NEXSTEP   0x11110001   /* |     |-- NeXTSTEP.             */
#define PLATFORM_OPENSTEP  0x11110002   /* |     |-- OPENSTEP.             */
#define PLATFORM_RHAPSODY  0x11110004   /* |     |-- Apple Rhapsody.       */
#define PLATFORM_MACOSX    0x11110008   /* |     `-- Apple Mac OS X.       */
#define PLATFORM_SVR3      0x12000000   /* |-- System V Release 3 family.  */
#define PLATFORM_AIX       0x12000001   /* | `-- IBM AIX.                  */
#define PLATFORM_SVR4      0x14000000   /* |-- System V Release 4 family.  */
#define PLATFORM_SOLARIS   0x14000001   /* | |-- Sun Solaris.              */
#define PLATFORM_HPUX      0x14000002   /* | `-- HP HP-UX >= 10.x.         */
#define PLATFORM_SVR5      0x18000000   /* `-- System V Release 5 family.  */
#define PLATFORM_VMS       0x20000000   /* DEC VMS.                        */

#if defined(__gnu_linux__) || defined(__linux__) || \
  defined(__linux) || defined(linux)
# define PLATFORM          PLATFORM_LINUX
# define PLATFORM_NAME     "GNU/Linux"
#elif defined(ultrix) || defined(__ultrix) || defined(__ultrix__)
/* Keep this before the BSDs... Ultrix defines BSD, too. */
# define PLATFORM          PLATFORM_ULTRIX
# define PLATFORM_NAME     "DEC Ultrix"
#elif defined(sun) || defined(__sun)
# if defined(__SVR4) || defined(__svr4__)
#  define PLATFORM         PLATFORM_SOLARIS
#  define PLATFORM_NAME    "Sun Microsystems Solaris"
# else
#  define PLATFORM         PLATFORM_SUNOS
#  define PLATFORM_NAME    "Sun Microsystems SunOS"
# endif
#elif defined(__MACH__)
# if defined(__NeXT__) || defined(NeXT)
#  if defined(NEXTSTEP)
#   define PLATFORM        PLATFORM_NEXTSTEP
#   define PLATFORM_NAME   "NeXTSTEP"
#  elif defined(OPENSTEP)
#   define PLATFORM        PLATFORM_OPENSTEP
#   define PLATFORM_NAME   "OPENSTEP"
#  elif defined(RHAPSODY)
#   define PLATFORM        PLATFORM_RHAPSODY
#   define PLATFORM_NAME   "Rhapsody"
#  endif
# elif defined(__APPLE__)
#  define PLATFORM         PLATFORM_MACOSX
#  define PLATFORM_NAME    "Apple OS X"
# endif
#elif defined(_AIX) || defined(__TOS_AIX__)
# define PLATFORM          PLATFORM_AIX
# define PLATFORM_NAME     "AIX"
#elif defined(__FreeBSD__)
# define PLATFORM          PLATFORM_FREEBSD
# define PLATFORM_NAME     "FreeBSD"
#elif defined(__NetBSD__)
# define PLATFORM          PLATFORM_NETBSD
# define PLATFORM_NAME     "NetBSD"
#elif defined(__OpenBSD__)
# define PLATFORM          PLATFORM_OPENBSD
# define PLATFORM_NAME     "OpenBSD"
#elif defined(__DragonFly__)
# define PLATFORM          PLATFORM_DRAGONFLY
# define PLATFORM_NAME     "DragonFly BSD"
#elif defined(__bsdi__)
# define PLATFORM          PLATFORM_BSDOS
# define PLATFORM_NAME     "BSD/OS"
#elif defined(_hpux) || defined(hpux) || defined(__hpux)
# if defined(BSD)
#  define PLATFORM         PLATFORM_HPUX9
# else
#  define PLATFORM         PLATFORM_HPUX
# endif
# define PLATFORM_NAME     "HP-UX"
#elif defined(__osf__) || defined(__osf)
# define PLATFORM          PLATFORM_OSF1
# define PLATFORM_NAME     "DEC OSF/1"
#elif defined(BSD)
# if (BSD == 42) || defined(BSD4_2)
#  define PLATFORM         PLATFORM_42BSD
#  define PLATFORM_NAME    "4.2 BSD"
# elif (BSD == 43) || defined(BSD4_3)
#  define PLATFORM         PLATFORM_43BSD
#  define PLATFORM_NAME    "4.3 BSD"
# elif (BSD == 44) || defined(BSD4_4)
#  define PLATFORM         PLATFORM_44BSD
#  define PLATFORM_NAME    "4.4 BSD"
# endif
#elif defined(VMS) || defined(__VMS)
# define PLATFORM          PLATFORM_VMS
# define PLATFORM_NAME     "VMS"
#else
# error "Unknown platform"
#endif

#define PLATFORM_EQ(__p)                        \
  ((PLATFORM & __p) == __p)

#define PLATFORM_GTE(__f,__c)                   \
  ((PLATFORM & __f) == __f) && (PLATFORM >= __c)

#define PLATFORM_LT(__f,__c)                    \
  ((PLATFORM & __f) == __f) && (PLATFORM <  __c)

#endif /* !_platform_h_ */

/* platform.h ends here. */
