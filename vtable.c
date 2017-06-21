/*
 * vtable.c --- vtable functions.
 *
 * Copyright (c) 2017 Paul Ward <asmodai@gmail.com>
 *
 * Author:     Paul Ward <asmodai@gmail.com>
 * Maintainer: Paul Ward <asmodai@gmail.com>
 * Created:    20 Jun 2017 21:53:02
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
 * @file vtable.c
 * @author Paul Ward
 * @brief vtable functions.
 */

#include <stdlib.h>
#include <string.h>

#include "vtable.h"
#include "sm_all.h"

void
generate_json(sm_base_t *inst)
{
  json_node_t *node = json_mkobject();

  inst->vtab->json_length = 0;

  if (inst->vtab->json_buffer != NULL) {
    free(inst->vtab->json_buffer);
  }

  if (inst->vtab->get_data != NULL) {
    (inst->vtab->get_data)(inst);
  }

  if (inst->vtab->emit_json != NULL) {
    (inst->vtab->emit_json)(&node);
    inst->vtab->json_buffer = json_stringify(node, NULL);
    inst->vtab->json_length = strlen(inst->vtab->json_buffer);
    json_delete(node);

    sm_all_update(inst);
  }
}

/* vtable.c ends here. */
