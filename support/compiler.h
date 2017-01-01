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

#define COMPILER_GCC        0x01000000  /* GNU C++ (g++) compiler.          */
#define COMPILER_GCC27      0x01000001  /* GCC 2.7.x.                       */
#define COMPILER_GCC30      0x01000002  /* GCC 3.0.x.                       */
#define COMPILER_UNKNOWN    0x80000000  /* Unknown or unsupported compiler. */


#if defined(__GNUC__)
# if (__GNUC__ == 2) && (__GNUC_MINOR__ < 7)
#  error "This compiler is too old!"
# elif (__GNUC__ == 2) && (__GNUC_MINOR__ == 7)
#  define COMPILER COMPILER_GCC27
# elif (__GNUC__ == 3) && (__GNUC_MINOR__ == 0)
#  define COMPILER COMPILER_GCC30
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

#endif /* !_compiler_h_ */

/* compiler.h ends here. */
