/*
 * compiler.h --- Compiler detection.
 *
 * Copyright (c) 2016 Paul Ward <asmodai@gmail.com>
 *
 * Author:     Paul Ward <asmodai@gmail.com>
 * Maintainer: Paul Ward <asmodai@gmail.com>
 * Created:    31 Dec 2016 08:34:30
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
 * @file compiler.h
 * @author Paul Ward
 * @brief Compiler detection.
 */

#ifndef _compiler_h_
#define _compiler_h_

/*
 * Compiler vendors and versions.
 */
#define COMPILER_GCC        0x01000000  /* GNU C++ (g++) compiler.          */
#define COMPILER_GCC27      0x01000001  /* GCC 2.x.                         */
#define COMPILER_GCC30      0x01000002  /* GCC 3.x.                         */
#define COMPILER_GCC40      0x01000100  /* GCC 4.x.                         */
#define COMPILER_GCC50      0x01001000  /* GCC 5.x.                         */
#define COMPILER_GCC60      0x01010000  /* GCC 6.x.                         */
#define COMPILER_UNKNOWN    0x80000000  /* Unknown or unsupported compiler. */

/* Detect compiler */
#if defined(__GNUC__)
# if (__GNUC__ == 2) && (__GNUC_MINOR__ < 7)
#  error "This compiler is too old!"
# elif (__GNUC__ == 2)
#  define COMPILER COMPILER_GCC27
# elif (__GNUC__ == 3)
#  define COMPILER COMPILER_GCC30
# elif (__GNUC__ == 4)
#  define COMPILER COMPILER_GCC40
# elif (__GNUC__ == 5)
#  define COMPILER COMPILER_GCC50
# elif (__GNUC__ == 6)
#  define COMPILER COMPILER_GCC60
# else
#  define COMPILER COMPILER_GCC
# endif
#elif
# define COMPILER COMPILER_UNKNOWN
#endif

#define COMPILER_EQ(__c) \
  ((COMPILER & __c) == __c)

#define COMPILER_GT(__f,__c) \
  ((COMPILER & __f) == __f) && (COMPILER >= __c)

#define COMPILER_LT(__f,__c) \
  ((COMPILER & __f) == __f) && (COMPILER <  __c)

/*
 * Standards.
 */
#define STANDARD_C89     0x00000001
#define STANDARD_C90     0x00000002
#define STANDARD_C95     0x00000004
#define STANDARD_C99     0x00000008
#define STANDARD_C11     0x00000010

#if defined(__STDC__)
# if defined(__STDC_VERSION__)
#  if (__STDC_VERSION__ >= 201112L)
#   define C_STANDARD  STANDARD_C11
#  elif (__STDC_VERSION__ >= 199901L)
#   define C_STANDARD  STANDARD_C99
#  elif (__STDC_VERSION__ >= 199409L)
#   define C_STANDARD  STANDARD_C95
#  else
#   define C_STANDARD  STANDARD_C90
#  endif
# else
#  define C_STANDARD   STANDARD_C89
# endif
#else
# error "You need an ANSI-capable C compiler!"
#endif

#endif /* !_compiler_h_ */

/* compiler.h ends here. */
