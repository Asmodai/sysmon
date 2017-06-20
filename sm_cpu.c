/*
 * sm_cpu.c --- CPU infomation.
 *
 * Copyright (c) 2017 Paul Ward <asmodai@gmail.com>
 *
 * Author:     Paul Ward <asmodai@gmail.com>
 * Maintainer: Paul Ward <asmodai@gmail.com>
 * Created:    19 Jun 2017 04:40:51
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
 * @file sm_cpu.c
 * @author Paul Ward
 * @brief CPU infomation.
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <time.h>

#include "json.h"
#include "vtable.h"
#include "sm_cpu.h"
#include "endpoints.h"
#include "timers.h"
#include "utils.h"

static endpoint_t   *cpu_endpoint   = NULL;
static sm_cpu_t     *cpu_instance   = NULL;
static timer_task_t *cpu_timer_task = NULL;

static const char strUnknown[]      = "unknown";
static const char strName[]         = "cpu";
static const char strNumConf[]      = "numConfigured";
static const char strNumOnln[]      = "numOnline";
static const char strClockSpeed[]   = "clockSpeed";
static const char strWordSize[]     = "wordSize";
static const char strArchitecture[] = "architecture";
static const char strModelName[]    = "model";
static const char strUpdated[]      = "updatedAt";

int
get_cpu(void *data)
{
  sm_cpu_t *ptr = (sm_cpu_t *)data;

  ptr->num_configured = sysconf(_SC_NPROCESSORS_CONF);
  ptr->num_online     = sysconf(_SC_NPROCESSORS_ONLN);
  ptr->clock_speed    = 0;
  ptr->word_size      = __WORDSIZE;
  ptr->architecture   = (char *)strUnknown;
  ptr->model          = (char *)strUnknown;
  ptr->time           = time(NULL);

  return 1;
}

void
cpu_timer(timer_clientdata_t data, struct timeval *now)
{
  printf("Updating CPU...\n");
  get_cpu((void *)cpu_instance);
}

void
emit_cpu(json_node_t **out)
{
  json_node_t *ret = out ? json_mkobject() : NULL;

  json_prepend_member(ret,
                      strNumConf,
                      json_mknumber(cpu_instance->num_configured));
  json_prepend_member(ret,
                      strNumOnln,
                      json_mknumber(cpu_instance->num_online));
  json_prepend_member(ret,
                      strClockSpeed,
                      json_mknumber(cpu_instance->clock_speed));
  json_prepend_member(ret,
                      strWordSize,
                      json_mknumber(cpu_instance->word_size));
  json_prepend_member(ret,
                      strArchitecture,
                      json_mkstring(cpu_instance->architecture));
  json_prepend_member(ret,
                      strModelName,
                      json_mkstring(cpu_instance->model));
  json_prepend_member(ret,
                      strUpdated,
                      json_mknumber(cpu_instance->time));

  if (out) {
    *out = ret;
  }
}

void
sm_cpu_init(void)
{
  if (cpu_instance == NULL) {
    cpu_instance       = xmalloc(sizeof(sm_cpu_t));
    cpu_instance->vtab = xmalloc(sizeof(sm_vtable_t));

    cpu_instance->vtab->get_data  = &get_cpu;
    cpu_instance->vtab->emit_json = &emit_cpu;

    /* We need to populate the struct with initial values. */
    get_cpu((void *)cpu_instance);
  }

  if (cpu_endpoint == NULL) {
    cpu_endpoint = endpoint_create(strName, cpu_instance);
  }

  if (cpu_timer_task == NULL) {
    timer_clientdata_t data;

    data.l = 0;
    cpu_timer_task = tmr_create(NULL,
                                &cpu_timer,
                                data,
                                10000,
                                1);
  }
}

/* sm_cpu.c ends here. */
