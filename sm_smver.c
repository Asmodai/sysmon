/*
 * sm_smver.c --- sysmon version.
 *
 * Copyright (c) 2016 Paul Ward <asmodai@gmail.com>
 *
 * Author:     Paul Ward <asmodai@gmail.com>
 * Maintainer: Paul Ward <asmodai@gmail.com>
 * Created:    30 Dec 2016 02:01:46
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
 * @file sm_smver.c
 * @author Paul Ward
 * @brief sysmon version.
 */

#include "config.h"

#include <stdlib.h>
#include <stdio.h>

#include "json.h"
#include "vtable.h"
#include "sm_smver.h"
#include "endpoints.h"
#include "version.h"
#include "utils.h"

static endpoint_t *smver_endpoint = NULL;
static sm_smver_t *smver_instance = NULL;

static const char strMajor[] = "major";
static const char strMinor[] = "minor";
static const char strPatch[] = "patch";
static const char strSmVer[] = "smver";

void
emit_smver(json_node_t **out)
{
  json_node_t *ret = out ? json_mkobject() : NULL;

  json_prepend_member(ret, strMajor,      json_mknumber(VERSION_MAJOR));
  json_prepend_member(ret, strMinor,      json_mknumber(VERSION_MINOR));
  json_prepend_member(ret, strPatch,      json_mknumber(VERSION_PATCH));

  if (out) {
    *out = ret;
  }
}

void
sm_smver_init(void)
{
  if (smver_instance == NULL) {
    smver_instance       = xmalloc(sizeof(sm_smver_t));
    smver_instance->vtab = xmalloc(sizeof(sm_vtable_t));

    smver_instance->vtab->get_data  = NULL;
    smver_instance->vtab->emit_json = &emit_smver;

    generate_json((sm_base_t *)smver_instance);
  }

  if (smver_endpoint == NULL) {
    smver_endpoint = endpoint_create(strSmVer, smver_instance);
  }
}

/* sm_smver.c ends here. */
