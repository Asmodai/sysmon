/*
 * sm_uname.c --- `uname` message.
 *
 * Copyright (c) 2016 Paul Ward <asmodai@gmail.com>
 *
 * Author:     Paul Ward <asmodai@gmail.com>
 * Maintainer: Paul Ward <asmodai@gmail.com>
 * Created:    22 Dec 2016 10:16:31
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
 * @file sm_uname.c
 * @author Paul Ward
 * @brief `uname` message.
 */

#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/param.h>

#ifdef HAVE_SYS_UTSNAME_H
# include <sys/utsname.h>
#else
# include "4bsd/utsname.h"
#endif

#include "utils.h"
#include "json.h"
#include "vtable.h"
#include "sm_uname.h"
#include "endpoints.h"

#if PLATFORM_LT(PLATFORM_BSD, PLATFORM_ULTRIX)
extern void perror(char *);
#endif

static endpoint_t *uname_endpoint = NULL;
static sm_uname_t *uname_instance = NULL;

static const char strSysName[]  = "sysname";
static const char strNodeName[] = "nodename";
static const char strRelease[]  = "release";
static const char strVersion[]  = "version";
static const char strMachine[]  = "machine";
static const char strName[]     = "uname";

void
emit_uname(json_node_t **out)
{
  struct utsname  name;
  json_node_t    *ret = out ? json_mkobject() : NULL;

  if ((uname(&name) != 0)) {
    perror("Unable to get uname from kernel");
    exit(1);
  }

  json_prepend_member(ret, strSysName,  json_mkstring(name.sysname));
  json_prepend_member(ret, strNodeName, json_mkstring(name.nodename));
  json_prepend_member(ret, strRelease,  json_mkstring(name.release));
  json_prepend_member(ret, strVersion,  json_mkstring(name.version));
  json_prepend_member(ret, strMachine,  json_mkstring(name.machine));

  if (out) {
    *out = ret;
  }
}

void
sm_uname_init(void)
{
  if (uname_instance == NULL) {
    uname_instance       = xmalloc(sizeof(sm_uname_t));
    uname_instance->vtab = xmalloc(sizeof(sm_vtable_t));

    uname_instance->vtab->get_data  = NULL;
    uname_instance->vtab->emit_json = &emit_uname;

    generate_json((sm_base_t *)uname_instance);
  }

  if (uname_endpoint == NULL) {
    uname_endpoint = endpoint_create(strName, uname_instance);
  }  
}

/* sm_uname.c ends here. */
