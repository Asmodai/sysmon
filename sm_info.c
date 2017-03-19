/*
 * sm_info.c --- Various info.
 *
 * Copyright (c) 2017 Paul Ward <asmodai@gmail.com>
 *
 * Author:     Paul Ward <asmodai@gmail.com>
 * Maintainer: Paul Ward <asmodai@gmail.com>
 * Created:    10 Mar 2017 20:13:40
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
 * @file sm_info.c
 * @author Paul Ward
 * @brief Various info.
 */

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>

#include "json.h"
#include "vtable.h"
#include "sm_info.h"
#include "endpoints.h"
#include "version.h"
#include "utils.h"

#include "support/compiler.h"
#include "support/posix.h"
#include "support/xopen.h"

static endpoint_t *info_endpoint = NULL;
static sm_info_t  *info_instance = NULL;

static const char strPosix[]    = "standardPOSIX";
static const char strXopen[]    = "standardXOpen";
static const char strCompiler[] = "build_compiler";
static const char strStdC[]     = "standardC";
static const char strSmInfo[]   = "info";

static char *posix_info = NULL;
static char *xopen_info = NULL;
static char *stdc_info  = NULL;

const size_t info_size = 128;

static
void
make_c_standard_info(void)
{
  if (stdc_info == NULL) {
    stdc_info = xmalloc(sizeof *stdc_info * info_size);
    bzero(stdc_info, sizeof *stdc_info * info_size);

    switch (C_STANDARD) {
      case STANDARD_C89:
        sprintf(stdc_info, "ANSI X3.159-1989 (C89)");
        break;

      case STANDARD_C90:
        sprintf(stdc_info, "ISO/IEC 9899:1990 (C90)");
        break;

      case STANDARD_C99:
        sprintf(stdc_info, "ISO/IEC 9899:1999 (C99)");
        break;
        
      case STANDARD_C11:
        sprintf(stdc_info, "ISO/IEC 9899:2011 (C11)");
        break;

      default:
        sprintf(stdc_info, "Not an ANSI C compiler");
        break;
    }
  }
}

static
void
make_posix_info(void)
{
  if (posix_info == NULL) {
    posix_info = xmalloc(sizeof *posix_info * info_size);
    bzero(posix_info, sizeof *posix_info * info_size);

    switch (POSIX_STANDARD) {
      case POSIX_1_1988:
        sprintf(posix_info, "POSIX.1-1988");
        break;

      case POSIX_1_1990:
        sprintf(posix_info, "ISO/IEC 9945-1:1990");
        break;

      case POSIX_2:
        sprintf(posix_info, "ISO/IEC 9945-2:1993");
        break;
      case POSIX_1B_1993:
        sprintf(posix_info, "IEEE 1003.1b-1993");
        break;

      case POSIX_1_1996:
        sprintf(posix_info, "IEEE 1003.1-1996");
        break;
      case POSIX_1_2001:
        sprintf(posix_info, "IEEE 1003.1-2001");
        break;

      case POSIX_1_2008:
        sprintf(posix_info, "IEEE 1003.1-2008");
        break;

      default:
        sprintf(posix_info, "No POSIX standard detected");
        break;
    }
  }
}

static
void
make_xopen_info(void)
{
  if (xopen_info == NULL) {
    xopen_info = xmalloc(sizeof *xopen_info * info_size);
    bzero(xopen_info, sizeof *xopen_info * info_size);

    switch (XOPEN_STANDARD) {
      case XOPEN_XPG3:
        sprintf(xopen_info, "X/Open Portability Guide 3");
        break;

      case XOPEN_XPG4:
        sprintf(xopen_info, "X/Open Portability Guide 4");
        break;

      case XOPEN_UNIX95:
        sprintf(xopen_info, "X/Open Single UNIX Specification v1");
        break;

      case XOPEN_UNIX98:
        sprintf(xopen_info, "X/Open Single UNIX Specification v2");
        break;

      case XOPEN_UNIX03:
        sprintf(xopen_info, "X/Open Single UNIX Specification v3");
        break;

      case XOPEN_SUS4:
        sprintf(xopen_info, "X/Open Single UNIX Specification v4");
        break;

      default:
        sprintf(xopen_info, "No X/Open standard detected");
        break;
    }
  }
}

void
emit_info(json_node_t **out)
{
  json_node_t *ret = out ? json_mkobject() : NULL;

  json_prepend_member(ret, strPosix,     json_mkstring(posix_info));
  json_prepend_member(ret, strXopen,     json_mkstring(xopen_info));
  json_prepend_member(ret, strStdC,     json_mkstring(stdc_info));
  json_prepend_member(ret, strCompiler,  json_mkstring(COMPILER_NAME));

  if (out) {
    *out = ret;
  }
}

void
sm_info_init(void)
{
  make_posix_info();
  make_xopen_info();
  make_c_standard_info();

  if (info_instance == NULL) {
    info_instance       = xmalloc(sizeof(sm_info_t));
    info_instance->vtab = xmalloc(sizeof(sm_vtable_t));

    info_instance->vtab->get_data  = NULL;
    info_instance->vtab->emit_json = &emit_info;
  }

  if (info_endpoint == NULL) {
    info_endpoint = endpoint_create(strSmInfo, info_instance);
  }
}

/* sm_info.c ends here. */
