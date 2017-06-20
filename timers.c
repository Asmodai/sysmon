/*
 * timers.c --- Timers implementation.
 *
 * Copyright (c) 2016 Paul Ward <asmodai@gmail.com>
 * Copyright (c) 1995,1998,1999,2000,2014 Jef Poskanzer <jef@mail.acme.com>
 *
 * Author:     Paul Ward <asmodai@gmail.com>
 * Maintainer: Paul Ward <asmodai@gmail.com>
 * Created:    29 Dec 2016 19:18:02
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
 * @file timers.c
 * @author Paul Ward
 * @brief Timers implementation.
 */

#include "config.h"

#include <sys/types.h>
#include <sys/time.h>

#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>

#include "timers.h"
#include "utils.h"

#if PLATFORM_EQ(PLATFORM_BSD)
# if PLATFORM_LT(PLATFORM_BSD, PLATFORM_BSDOS)
#  if PLATFORM_GTE(PLATFORM_BSD, PLATFORM_ULTRIX)
extern int  gettimeofday(struct timeval *, struct timezone *);
extern void syslog(int, char *, ...);
#  else
extern int  gettimeofday();
extern int  syslog();
extern int  free();
#  endif  /* BSD > ULTRIX */
# endif  /* BSD < BSDOS */
#endif

#define HASH_SIZE 67

static timer_task_t *timers[HASH_SIZE];
static timer_task_t *free_timers;
static size_t        timers_alloc_count;
static size_t        timers_active_count;
static size_t        timers_free_count;

timer_clientdata_t JunkClientData;

static
unsigned int
hash(timer_task_t *t)
{
  return ((unsigned int)t->time.tv_sec ^ (unsigned int)t->time.tv_usec) %
      HASH_SIZE;
}

static
void
l_add(timer_task_t *t)
{
  int           h      = t->hash;
  timer_task_t *t2     = NULL;
  timer_task_t *t2prev = NULL;

  t2 = timers[h];
  if (t2 == NULL) {
    timers[h] = t;
    t->prev   = t->next = NULL;
  } else {
    if (t->time.tv_sec < t2->time.tv_sec ||
        (t->time.tv_sec  == t2->time.tv_sec &&
         t->time.tv_usec <= t2->time.tv_usec))
    {
      timers[h] = t;
      t->prev   = NULL;
      t->next   = t2;
      t2->prev  = t;
    } else {
      for (t2prev = t2, t2 = t2->next; t2 != NULL; t2prev = t2, t2 = t2->next)
      {
        if (t->time.tv_sec < t2->time.tv_sec ||
            (t->time.tv_sec  == t2->time.tv_sec &&
             t->time.tv_usec <= t2->time.tv_usec))
        {
          t2prev->next = t;
          t->prev      = t2prev;
          t->next      = t2;
          t2->prev     = t;

          return;
        }
      }

      t2prev->next = t;
      t->prev      = t2prev;
      t->next      = NULL;
    }
  }
}

static
void
l_remove(timer_task_t *t)
{
  int h = t->hash;

  if (t->prev == NULL) {
    timers[h] = t->next;
  } else {
    t->prev->next = t->next;
  }

  if (t->next != NULL) {
    t->next->prev = t->prev;
  }
}

static
void
l_resort(timer_task_t *t)
{
  l_remove(t);
  t->hash = hash(t);
  l_add(t);
}

void
tmr_init(void)
{
  int h = 0;

  for (h = 0; h < HASH_SIZE; ++h) {
    timers[h] = NULL;
  }

  free_timers = NULL;
  timers_alloc_count = timers_active_count = timers_free_count = 0;
}

timer_task_t *
tmr_create(struct timeval     *now,
           timer_proc_t       *timer_proc,
           timer_clientdata_t  client_data,
           long                msecs,
           int                 periodic)
{
  timer_task_t *t = NULL;

  if (free_timers != NULL) {
    t           = free_timers;
    free_timers = t->next;
    timers_free_count--;
  } else {
    t = (timer_task_t *)xmalloc(sizeof(timer_task_t));

    timers_alloc_count++;
  }

  t->timer_proc  = timer_proc;
  t->client_data = client_data;
  t->msecs       = msecs;
  t->periodic    = periodic;

  if (now != NULL) {
    t->time = *now;
  } else {
    gettimeofday(&t->time, (struct timezone *)0);
  }

  t->time.tv_sec  += msecs / 1000L;
  t->time.tv_usec += (msecs % 1000L) * 1000L;

  if (t->time.tv_usec >= 1000000L) {
    t->time.tv_sec    += t->time.tv_usec / 1000000L;
    t->time.tv_usec   %= 1000000L;
  }

  t->hash = hash(t);
  l_add(t);
  timers_active_count++;

  return t;
}

struct timeval *
tmr_task_timeout(struct timeval *now)
{
  long                  msecs = 0;
  static struct timeval timeout;

  msecs = tmr_mstimeout(now);

  if (msecs == INFTIM) {
    return NULL;
  }

  timeout.tv_sec  = msecs / 1000L;
  timeout.tv_usec = (msecs % 1000L) * 1000L;

  return &timeout;
}

long
tmr_mstimeout(struct timeval *now)
{
  int           h      = 0;
  int           gotone = 0;
  long          msecs  = 0;
  long          m      = 0;
  timer_task_t *t      = NULL;

  for (h = 0; h < HASH_SIZE; ++h)	{
    t = timers[h];

    if (t != NULL) {
      m = (t->time.tv_sec  - now->tv_sec) * 1000L +
          (t->time.tv_usec - now->tv_usec) / 1000L;

      if (!gotone) {
        msecs  = m;
        gotone = 1;
      } else if (m < msecs) {
        msecs = m;
      }
    }
	}

  if (!gotone) {
    return INFTIM;
  }

  if (msecs <= 0) {
    msecs = 0;
  }

  return msecs;
}

void
tmr_run(struct timeval *now)
{
  int           h    = 0;
  timer_task_t *t    = NULL;
  timer_task_t *next = NULL;

  for (h = 0; h < HASH_SIZE; ++h) {
    for (t = timers[h]; t != NULL; t = next) {
	    next = t->next;

	    if (t->time.tv_sec > now->tv_sec ||
          (t->time.tv_sec == now->tv_sec &&
           t->time.tv_usec > now->tv_usec))
      {
        break;
      }

	    (t->timer_proc)(t->client_data, now);
	    if ( t->periodic )
      {
        /* Reschedule. */
        t->time.tv_sec  += t->msecs / 1000L;
        t->time.tv_usec += (t->msecs % 1000L) * 1000L;

        if (t->time.tv_usec >= 1000000L) {
          t->time.tv_sec  += t->time.tv_usec / 1000000L;
          t->time.tv_usec %= 1000000L;
        }

        l_resort(t);
      } else {
        tmr_cancel(t);
      }
    }
  }
}

void
tmr_reset(struct timeval *now, timer_task_t *t)
{
  t->time          = *now;
  t->time.tv_sec  += t->msecs / 1000L;
  t->time.tv_usec += (t->msecs % 1000L) * 1000L;

  if (t->time.tv_usec >= 1000000L) {
    t->time.tv_sec  += t->time.tv_usec / 1000000L;
    t->time.tv_usec %= 1000000L;
  }

  l_resort(t);
}

void
tmr_cancel(timer_task_t *t)
{
  l_remove(t);
  timers_active_count--;

  t->next     = free_timers;
  free_timers = t;
  timers_free_count++;

  t->prev = NULL;
}

void
tmr_cleanup(void)
{
  timer_task_t *t = NULL;

  while (free_timers != NULL) {
    t           = free_timers;
    free_timers = t->next;

    timers_free_count--;
    free(t);
    timers_alloc_count--;
  }
}

void
tmr_task_term(void)
{
  int h = 0;

  for (h = 0; h < HASH_SIZE; h++) {
    tmr_cancel(timers[h]);
  }

  tmr_cleanup();
}

void
tmr_logstats(long secs)
{
  syslog(LOG_NOTICE,
         "timers - %lu allocated, %lu active, %lu free",
         timers_alloc_count,
         timers_active_count,
         timers_free_count);

  if (timers_active_count + timers_free_count != timers_alloc_count) {
    syslog(LOG_ERR, "Timer counts do not add up!");
  }
}

/* timers.c ends here. */
