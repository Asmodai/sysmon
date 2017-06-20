/*
 * timers.h --- Timers.
 *
 * Copyright (c) 2016 Paul Ward <asmodai@gmail.com>
 * Copyright (c) 1995,1998,1999,2000,2014 Jef Poskanzer <jef@mail.acme.com>
 *
 * Author:     Paul Ward <asmodai@gmail.com>
 * Maintainer: Paul Ward <asmodai@gmail.com>
 * Created:    29 Dec 2016 19:05:36
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
 * @file timers.h
 * @author Paul Ward
 * @brief Timers.
 */

#ifndef _timers_h_
#define _timers_h_

#include <sys/time.h>

#ifndef INFTIM
# define INFTIM -1
#endif

typedef union {
  void *p;
  int   i;
  long  l;
} timer_clientdata_t;

extern timer_clientdata_t JunkClientData;

typedef void timer_proc_t(timer_clientdata_t cd, struct timeval *now);

typedef struct timer_task_task_s {
  timer_proc_t             *timer_proc;
  timer_clientdata_t        client_data;
  long                      msecs;
  int                       periodic;
  struct timeval            time;
  struct timer_task_task_s *prev;
  struct timer_task_task_s *next;
  int                       hash;
} timer_task_t;

void            tmr_init(void);
timer_task_t   *tmr_create(struct timeval     *now,
                           timer_proc_t       *timer_proc,
                           timer_clientdata_t  client_data,
                           long                msecs,
                           int                 periodic);
struct timeval *tmr_task_timeout(struct timeval *now);
long            tmr_mstimeout(struct timeval *now);
void            tmr_run(struct timeval *now);
void            tmr_reset(struct timeval *now, timer_task_t *timer);
void            tmr_cancel(timer_task_t *timer);
void            tmr_cleanup(void);
void            tmr_task_term(void);
void            tmr_logstats(long secs);

#endif /* !_timers_h_ */

/* timers.h ends here. */
