/*
 * vtable.h --- vtable structure.
 *
 * Copyright (c) 2016 Paul Ward <asmodai@gmail.com>
 *
 * Author:     Paul Ward <asmodai@gmail.com>
 * Maintainer: Paul Ward <asmodai@gmail.com>
 * Created:    29 Dec 2016 22:25:56
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
 * @file vtable.h
 * @author Paul Ward
 * @brief vtable structure.
 */

#ifndef _vtable_h_
#define _vtable_h_

#include "json.h"

#define MAKE_VTABLE(__inst, __get, __emit, __once) \
  (__inst)->vtab->get_data    = (__get);           \
  (__inst)->vtab->emit_json   = (__emit);          \
  (__inst)->vtab->only_once   = (__once);          \
  (__inst)->vtab->json_buffer = NULL;              \
  (__inst)->vtab->json_length = 0;                 \
  (__inst)->vtab->done_once   = 0

/*
 * Virtual function table.
 */
typedef struct {
  void   (*get_data)(void *);
  void   (*emit_json)(json_node_t **);
  char    *json_buffer;
  size_t   json_length;
  int      only_once;
  int      done_once;
} sm_vtable_t;

/*
 * The word `base` here isn't really true in the OO sense.  Rather, this abuses
 * polymorphism to allow us to invoke a vtable method without having to care
 * about using an explicit structure (such as sm_uname_t).
 */
typedef struct {
  sm_vtable_t *vtab;                    /* Virtual function table. */
} sm_base_t;

void generate_json(sm_base_t *);

#endif /* !_vtable_h_ */

/* vtable.h ends here. */
