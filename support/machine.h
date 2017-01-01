/*
 * machine.h --- Machine type detection.
 *
 * Copyright (c) 2016 Paul Ward <asmodai@gmail.com>
 *
 * Author:     Paul Ward <asmodai@gmail.com>
 * Maintainer: Paul Ward <asmodai@gmail.com>
 * Created:    31 Dec 2016 21:48:44
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
 * @file machine.h
 * @author Paul Ward
 * @brief Machine type detection.
 */

#ifndef _machine_h_
#define _machine_h_

#define ARCHITECTURE_AXP       1        /* DEC Alpha.                 */
#define ARCHITECTURE_AMD64     2        /* Intel/AMD 64-bit.          */
#define ARCHITECTURE_ARM       3        /* Acorn RISC Machine         */
#define ARCHITECTURE_ARM64     4        /* 64-bit ARM.                */
#define ARCHITECTURE_HPPA      5        /* HP PA-RISC.                */
#define ARCHITECTURE_IA32      6        /* Intel 32-bit architecture. */
#define ARCHITECTURE_IA64      7        /* Intel 64-bit architecture. */
#define ARCHITECTURE_M68K      8        /* Motorola 680x0.            */
#define ARCHITECTURE_M88K      9        /* Motorola 88000.            */
#define ARCHITECTURE_MIPS      10       /* MIPS architecture.         */
#define ARCHITECTURE_POWERPC   11       /* Motorola PowerPC           */
#define ARCHITECTURE_POWER     12       /* IBM POWER.                 */
#define ARCHITECTURE_SPARC     13       /* Sun Microsystems SPARC.    */
#define ARCHITECTURE_SYSTEMZ   14       /* IBM SystemZ.               */
#define ARCHITECTURE_VAX       15       /* DEC VAX.                   */
#define ARCHITECTURE_UNKNOWN   99       /* Unknown architecture.      */

/*
 * If you need to add to this, check the output of
 *
 *    cpp -dM </dev/null
 *
 * for architecture macros.
 */
#if defined(__alpha__) || defined(__alpha)
# define ARCHITECTURE      ARCHITECTURE_AXP
# define ARCHITECTURE_NAME "Alpha"
#elif defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || \
  defined(__x86_64)
# define ARCHITECTURE      ARCHITECTURE_AMD64
# define ARCHITECTURE_NAME "AMD64"
#elif defined(__arm__) || defined(__thumb__) || defined(__arm)
# define ARCHITECTURE      ARCHITECTURE_ARM
# define ARCHITECTURE_NAME "ARM"
#elif defined(__arch64__)
# define ARCHITECTURE      ARCHITECTURE_ARM64
# define ARCHITECTURE_NAME "ARM64"
#elif defined(__hppa__) || defined(__HPPA__) || defined(__hppa)
# define ARCHITECTURE      ARCHITECTURE_HPPA
# define ARCHITECTURE_NAME "PA-RISC"
#elif defined(i386) || defined(__i386) || defined(__i386__)
# define ARCHITECTURE      ARCHITECTURE_IA32
# define ARCHITECTURE_NAME "IA32"
#elif defined(__ia64__) || defined(_IA64) || defined(__IA64) || \
  defined(__ia64) || defined(__itanium__)
# define ARCHITECTURE      ARCHITECTURE_IA64
# define ARCHITECTURE_NAME "IA64"
#elif defined(__m68k__)
# define ARCHITECTURE      ARCHITECTURE_M68K
# define ARCHITECTURE_NAME "M68000"
#elif defined(__m88k__)
# define ARCHITECTURE      ARCHITECTURE_M88K
# define ARCHITECTURE_NAME "M88000"
#elif defined(__mips__) || defined(mips)
# define ARCHITECTURE      ARCHITECTURE_MIPS
# define ARCHITECTURE_NAME "MIPS"
#elif defined(__powerpc__) || defined(__powerpc) || defined(__powerpc64__) || \
  defined(__POWERPC__) || defined(__ppc__) || defined(__ppc64__) || \
  defined(__PPC__) || defined(__PPC64__) || defined(_ARCH_PPC) || \
  defined(_ARCH_PPC64)
# define ARCHITECTURE      ARCHITECTURE_POWERPC
# define ARCHITECTURE_NAME "PowerPC"
#elif defined(__THW_RS6000) || defined(__IBMR2) || defined(_POWER)
# define ARCHITECTURE      ARCHITECTURE_POWER
# define ARCHITECTURE_NAME "POWER"
#elif defined(__sparc__) || defined(__sparc)
# define ARCHITECTURE      ARCHITECTURE_SPARC
# define ARCHITECTURE_NAME "SPARC"
#elif defined(__370__) || defined(__THW_370__) || defined(__s390__) || \
  defined(__s390x__) || defined(__zarch__) || defined(__SYSC_ZARCH__)
# define ARCHITECTURE      ARCHITECTURE_SYSTEMZ
# define ARCHITECTURE_NAME "SystemZ"
#elif defined(vax) || defined(__vax) || defined(__vax__)
# define ARCHITECTURE      ARCHITECTURE_VAX
# define ARCHITECTURE_NAME "VAX"
#else
# error "Unknown machine"
#endif

/* Special cases for fat NeXT platforms. */
#if defined(NeXT)
# if (__ARCHITECTURE == "i386")
#  define ARCHITECTURE      ARCHITECTURE_IA32
#  define ARCHITECTURE_NAME "IA32"
# elif (__ARCHITECTURE == "m68k")
#  define ARCHITECTURE      ARCHITECTURE_M68K
#  define ARCHITECTURE_NAME "M68000"
# elif (__ARCHITECTURE == "sparc")
#  define ARCHITECTURE      ARCHITECTURE_SPARC
#  define ARCHITECTURE_NAME "SPARC"
# elif (__ARCHITECTURE == "ppc")
#  define ARCHITECTURE      ARCHITECTURE_POWERPC
#  define ARCHITECTURE_NAME "PowerPC"
# elif (__ARCHITECTURE == "hppa")
#  define ARCHITECTURE      ARCHITECTURE_HPPA
#  define ARCHITECTURE_NAME "HPPA"
# endif
#endif

#define ARCHITECTURE_EQ(__p)                         \
  (ARCHITECTURE == __p)

#endif /* !_machine_h_ */

/* machine.h ends here. */
