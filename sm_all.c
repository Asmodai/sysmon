/*
 * sm_all.c --- Iterate all the things.
 *
 * Copyright (c) 2016 Paul Ward <asmodai@gmail.com>
 *
 * Author:     Paul Ward <asmodai@gmail.com>
 * Maintainer: Paul Ward <asmodai@gmail.com>
 * Created:    30 Dec 2016 02:32:58
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
 * @file sm_all.c
 * @author Paul Ward
 * @brief Iterate all the things.
 */

#include <stdlib.h>
#include <stdio.h>

#include "json.h"
#include "vtable.h"
#include "sm_all.h"
#include "endpoints.h"
#include "version.h"
#include "utils.h"

static endpoint_t *all_endpoint = NULL;
static sm_all_t   *all_instance = NULL;

void
emit_all(json_node_t **out)
{
  json_node_t       *ret  = out ? json_mkobject() : NULL;
  endpoint_t        *node = NULL;
  extern endpoint_t *endpoints;

  endpoint_foreach(node) {
    json_node_t *obj  = NULL;
    sm_base_t   *inst = NULL;

    /* Infinite loops are bad. */
    if (node->hash == all_endpoint->hash) {
      continue;
    }

    if (node != NULL) {
      inst = (sm_base_t *)node->instance;

      (inst->vtab->emit_json)(&obj);

      if (obj != NULL) {
        json_append_member(ret, node->name, obj);
      }
    }
  }

  if (out) {
    *out = ret;
  }
}

void
sm_all_init(void)
{
  if (all_instance == NULL) {
    all_instance       = xmalloc(sizeof(sm_all_t));
    all_instance->vtab = xmalloc(sizeof(sm_vtable_t));

    all_instance->vtab->get_data  = NULL;
    all_instance->vtab->emit_json = &emit_all;

    all_instance->vtab->json_buffer = NULL;
    all_instance->vtab->json_length = 0;
  }

  if (all_endpoint == NULL) {
    all_endpoint = endpoint_create("all", all_instance);
  }
}

void
sm_all_update(sm_base_t *endpoint)
{
  if (endpoint     == (sm_base_t *)all_instance ||
      all_instance == NULL                  ||
      endpoint     == NULL)
  {
    return;
  }

  generate_json((sm_base_t *)all_instance);
}

/* sm_all.c ends here. */
