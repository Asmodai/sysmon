/*
 * main.c --- Main file.
 *
 * Copyright (c) 2016 Paul Ward <asmodai@gmail.com>
 *
 * Author:     Paul Ward <asmodai@gmail.com>
 * Maintainer: Paul Ward <asmodai@gmail.com>
 * Created:    23 Dec 2016 14:47:18
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
 * @file main.c
 * @author Paul Ward
 * @brief Main file.
 */

#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# include "posix/stdbool.h"
#endif

#include "json.h"
#include "endpoints.h"
#include "timers.h"
#include "sm_uname.h"
#include "sm_smver.h"
#include "sm_all.h"

bool Pretty_Output;

int
main(void)
{
  endpoint_t *node = NULL;

  timer_init();
  endpoint_init();

  Pretty_Output = false;

  sm_uname_init();
  sm_smver_init();
  sm_all_init();

  node = endpoint_find("all");
  if (node != NULL) {
    sm_uname_t *inst = (sm_uname_t *)node->instance;
    json_node_t *obj = json_mkobject();

    (inst->vtab->emit_json)(&obj);

    printf("%s\n", json_stringify(obj, NULL));

    free(obj);
  }

  return 0;
}

/* main.c ends here. */
