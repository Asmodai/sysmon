/*
 * fdwatch.c --- File descriptor watching.
 *
 * Copyright (c) 2017 Paul Ward <asmodai@gmail.com>
 * Copyright (c) 1999 Jef Poskanzer <jef@mail.acme.com>
 *
 * Author:     Paul Ward <asmodai@gmail.com>
 * Maintainer: Paul Ward <asmodai@gmail.com>
 * Created:    01 Jan 2017 06:00:42
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
 * @file fdwatch.c
 * @author Paul Ward
 * @brief File descriptor watching.
 */

#include "config.h"

#include <sys/types.h>
#include <sys/time.h>

#if !PLATFORM_LT(PLATFORM_BSD, PLATFORM_44BSD)
# include <sys/resource.h>
#endif

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <fcntl.h>

#include "utils.h"

#if PLATFORM_EQ(PLATFORM_LINUX)
# define HAVE_POLL_H
# define HAVE_POLL
#elif PLATFORM_EQ(PLATFORM_BSD)
# if PLATFORM_LT(PLATFORM_BSD, PLATFORM_BSDOS)
#  define HAVE_SELECT
# endif
#else
# define HAVE_SELECT
#endif

#ifdef HAVE_POLL_H
# include <poll.h>
#else
# ifdef HAVE_SYS_POLL_H
#  include <sys/poll.h>
# endif
#endif

#ifdef HAVE_SYS_DEVPOLL_H
# include <sys/devpoll.h>
# ifndef HAVE_DEVPOLL
#  define HAVE_DEVPOLL
# endif
#endif

#ifdef HAVE_SYS_EVENT_H
# include <sys/event.h>
#endif

#include "fdwatch.h"
#include "utils.h"

#if PLATFORM_EQ(PLATFORM_BSD)
# if PLATFORM_LT(PLATFORM_BSD, PLATFORM_BSDOS)
extern int  getdtablesize();
#  if PLATFORM_GTE(PLATFORM_BSD, PLATFORM_ULTRIX)
extern void syslog(int, char *, ...);
extern int  select();
extern void bzero();
#  else
extern int  syslog();
extern int  bzero();
extern int  select();
#  endif  /* BSD > ULTRIX */
# endif  /* BSD < BSDOS */
#endif

#ifdef HAVE_SELECT
# ifndef FD_SET
#  define NFDBITS     32
#  define FD_SETSIZE  32
#  define FD_SET(n, p)   ((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#  define FD_CLR(n, p)   ((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#  define FD_ISSET(n, p) ((p)->fds_bits[(n)/NFDBITS]] & (1 << ((n) % NFDBITS)))
#  define FD_ZERO(p)     bzero((char *)(p), sizeof(*(p)))
# endif
#endif

static int    nfiles;
static long   nwatches;
static int   *fd_rw;
static void **fd_data;
static int    nreturned;
static int    next_ridx;

#if defined(HAVE_KQUEUE)

# define WHICH              "kevent"
# define INIT(nf)           kqueue_init(nf)
# define ADD_FD(fd, rw)     kqueue_add_fd(fd, rw)
# define DEL_FD(fd)         kqueue_del_fd(fd)
# define WATCH(tmout)       kqueue_watch(tmout)
# define CHECK_FD(fd)       kqueue_check_fd(fd)
# define GET_FD(ridx)       kqueue_get_fd(ridx)

static int  kqueue_init(int nf);
static void kqueue_add_fd(int fd, int rw);
static void kqueue_del_fd(int fd);
static int  kqueue_watch(long tmout);
static int  kqueue_check_fd(int fd);
static int  kqueue_get_fd(int ridx);

#elif defined(HAVE_DEVPOLL)

# define WHICH              "devpoll"
# define INIT(nf)           devpoll_init(nf)
# define ADD_FD(fd, rw)     devpoll_add_fd(fd, rw)
# define DEL_FD(fd)         devpoll_del_fd(fd)
# define WATCH(tmout)       devpoll_watch(tmout)
# define CHECK_FD(fd)       devpoll_check_fd(fd)
# define GET_FD(ridx)       devpoll_get_fd(ridx)

static int  devpoll_init(int nf);
static void devpoll_add_fd(int fd, int rw);
static void devpoll_del_fd(int fd);
static int  devpoll_watch(long tmout);
static int  devpoll_check_fd(int fd);
static int  devpoll_get_fd(int ridx);

#elif defined(HAVE_POLL)

# define WHICH              "poll"
# define INIT(nf)           poll_init(nf)
# define ADD_FD(fd, rw)     poll_add_fd(fd, rw)
# define DEL_FD(fd)         poll_del_fd(fd)
# define WATCH(tmout)       poll_watch(tmout)
# define CHECK_FD(fd)       poll_check_fd(fd)
# define GET_FD(ridx)       poll_get_fd(ridx)

static int  poll_init(int nf);
static void poll_add_fd(int fd, int rw);
static void poll_del_fd(int fd);
static int  poll_watch(long tmout);
static int  poll_check_fd(int fd);
static int  poll_get_fd(int ridx);

#elif defined(HAVE_SELECT)

# define WHICH              "select"
# define INIT(nf)           select_init(nf)
# define ADD_FD(fd, rw)     select_add_fd(fd, rw)
# define DEL_FD(fd)         select_del_fd(fd)
# define WATCH(tmout)       select_watch(tmout)
# define CHECK_FD(fd)       select_check_fd(fd)
# define GET_FD(ridx)       select_get_fd(ridx)

static int  select_init(int nf);
static void select_add_fd(int fd, int rw);
static void select_del_fd(int fd);
static int  select_watch(long tmout);
static int  select_check_fd(int fd);
static int  select_get_fd(int ridx);

#else
# error "Do not know how to watch file descriptors on this system."
#endif

int
fdwatch_get_nfiles(void)
{
  int i = 0;

#ifdef RLIMIT_NOFILE
  struct rlimit rl;
#endif

  nfiles = getdtablesize();

#ifdef RLIMIT_NOFILE
  if (getrlimit(RLIMIT_NOFILE, &rl) == 0) {
    nfiles = rl.rlim_cur;

    if (rl.rlim_max == RLIM_INFINITY) {
      rl.rlim_cur = 8192;
    } else if (rl.rlim_max > rl.rlim_cur) {
      rl.rlim_cur = rl.rlim_max;
    }

    if (setrlimit(RLIMIT_NOFILE, &rl) == 0) {
      nfiles = rl.rlim_cur;
    }
  }
#endif

#if defined(HAVE_SELECT) && \
  !(defined(HAVE_POLL) || defined(HAVE_DEVPOLL) || defined(HAVE_KQUEUE))
  nfiles = MIN(nfiles, FD_SETSIZE);
#endif

  nwatches = 0;
  fd_rw    = xmalloc(sizeof(int) * nfiles);
  fd_data  = xmalloc(sizeof(void *) * nfiles);

  for (i = 0; i < nfiles; ++i) {
    fd_rw[i] = -1;
  }

  if (INIT(nfiles) == -1) {
    return -1;
  }

  return nfiles;
}

void
fdwatch_add_fd(int fd, void *client_data, int rw)
{
  if (fd < 0 || fd >= nfiles || fd_rw[fd] != -1) {
    syslog(LOG_ERR, "bad fd (%d) passed  to fdwatch_add_fd!", fd);
    return;
  }

  ADD_FD(fd, rw);
  fd_rw[fd]   = rw;
  fd_data[fd] = client_data;
}

void
fdwwatch_del_fd(int fd)
{
  if (fd < 0 || fd >= nfiles || fd_rw[fd] == -1) {
    syslog(LOG_ERR, "bad fd (%d) passed to fdwatch_del_fd!", fd);
    return;
  }

  DEL_FD(fd);
  fd_rw[fd]   = -1;
  fd_data[fd] = NULL;
}

int
fdwatch(long tmout)
{
  nwatches++;
  nreturned = WATCH(tmout);
  next_ridx = 0;

  return nreturned;
}

int
fdwatch_check_fd(int fd)
{
  if (fd < 0 || fd >= nfiles || fd_rw[fd] == -1) {
    syslog(LOG_ERR, "bad fd (%d) passed to fdwatch_check_fd!", fd);
    return 0;
  }

  return CHECK_FD(fd);
}

void *
fdwatch_get_next_client_data(void)
{
  int fd = 0;

  if (next_ridx >= nreturned) {
    return (void *)-1;
  }

  fd = GET_FD(next_ridx++);

  if (fd < 0 || fd >= nfiles) {
    return NULL;
  }

  return fd_data[fd];
}

void
fdwatch_logstats(long secs)
{
  if (secs > 0) {
    syslog(LOG_NOTICE,
           "fdwatch - %ld %ss (%g/sec)",
           nwatches,
           WHICH,
           (float)nwatches / secs);
  }

  nwatches = 0;
}

#if defined(HAVE_KQUEUE)
/* ========================================================================== */
/* {{{ `kqueue` implementation: */

static int            maxkqevents;
static struct kevent *kqevents;
static int            nkqevents;
static struct kevent *kqrevents;
static int           *kqrfdidx;
static int            kq;

static
int
kqueue_init(int nf)
{
  kq = kqueue();
  if (kq == -1) {
    return -1
  }

  maxkqevents = nf * 2;
  kqevents    = xmalloc(sizeof(struct kevent) * maxkqevents);
  kqrevents   = xmalloc(sizeof(struct kevent) * nf);
  kqrfdidx    = xmalloc(sizeof(int) * nf);

  memset(kqevents, 0, sizeof(struct kevent) * maxkqevents);
  memset(kqrfdidx, 0, sizeof(int) * nf);

  return 0;
}

static
void
kqueue_add_fd(int fd, int rw)
{
  if (nkqevents >= maxkqevents) {
    syslog(LOG_ERR, "too many kqevents in kqueue_add_fd!");
    return;
  }

  kqevents[nkqevents].ident = fd;
  kqevents[nkqevents].flags = EV_ADD;

  switch (rw) {
    case FDW_READ:  kqevents[nkqevents].filter = EVFILT_READ;  break;
    case FDW_WRITE: kqevents[nkqevents].filter = EVFILT_WRITE; break;
    default:                                                   break;
  }

  nkqevents++;
}

static
void
kqueue_del_fd(int fd)
{
  if (nkqevents >= maxkqevents) {
    syslog(LOG_ERR, "too many kqevents in kqueue_del_fd!");
    return;
  }

  kqevents[nkqevents].ident = fd;
  kqevents[nkqevents].flags = EV_DELETE;

  switch (fd_rw[fd]) {
    case FDW_READ:  kqevents[nkqevents].filter = EVFILT_READ;  break;
    case FDW_WRITE: kqevents[nkqevents].filter = EVFILT_WRITE; break;
    default:                                                   break;
  }

  nkqevents++;
}

static
int
kqueue_watch(long tmout)
{
  int i = 0;
  int r = 0;

  if (tmout == INFTIM) {
    r = kevent(kq,
               kqevents,
               nkqevents,
               nfiles,
               NULL);
  } else {
    struct timespec ts;

    ts.tv_sec  = tmout / 1000L;
    ts.tv_nsec = (tmout % 1000L) * 1000000L;
    r          = kevent(kq,
                        kqevents,
                        nkqevents,
                        kqrevents,
                        nfiles,
                        &ts);
  }

  nkqevents = 0;

  if (r == -1) {
    return -1;
  }

  for (i = 0; i < r; ++i) {
    kqrfdidx[kqrevents[i].ident] = i;
  }

  return r;
}

static
int
kqueue_check_fd(int fd)
{
  int ridx = kqrfdidx[fd];

  if (ridx < 0 || ridx >= nfiles) {
    syslog(LOG_ERR, "bad ridx (%d) in kqueue_check_fd!", ridx);
    return 0;
  }

  if (ridx >= nreturned) {
    return 0;
  }

  if (kqrevents[ridx].ident != fd) {
    return 0;
  }

  if (kqrevents[ridx].flags & EV_ERROR) {
    return 0;
  }

  switch (fd_rw[fd]) {
    case FDW_READ:  return kqrevents[ridx].filter == EVFILT_READ;
    case FDW_WRITE: return kqrevents[ridx].filter == EVFILT_WRITE;
    default:        return 0;
  }
}

static
int
kqueue_get_fd(int ridx)
{
  if (ridx < 0 || ridx >= nfiles) {
    syslog(LOG_ERR, "bad ridx (%d) in kqueue_get_fd!", ridx);
    return -1;
  }

  return kqrevents[ridx].ident;
}

/* }}} */
/* ========================================================================== */
#elif defined(HAVE_DEVPOLL)
/* ========================================================================== */
/* {{{ `devpoll` implementation: */

static int            maxdpevents;
static struct pollfd *dpevents;
static int            ndpevents;
static struct pollfd *dprevents;
static int           *dp_rfdidx;
static int            dp;

static
int
devpoll_init(int nf)
{
  if ((dp = open("/dev/poll", O_RDWR)) == -1) {
    return -1;
  }

  fcntl(dp, F_SETFD, 1);

  maxdpevents = nf * 2;
  dpevents    = xmalloc(sizeof(struct pollfd) * maxdpevents);
  dprevents   = xmalloc(sizeof(struct pollfd) * nf);
  dp_rfdidx   = xmalloc(sizeof(int) * nf);
  memset(dp_rfdidx, 0, sizeof(int) * nf);

  return 0;
}

static
void
devpoll_add_fd(int fd, int rw)
{
  if (ndpevents >= maxdpevents) {
    syslog(LOG_ERR, "too many fds in devpoll_add_fd!");
    return;
  }

  dpevents[ndpevents].fd = fd;

  switch (rw) {
    case FDW_READ:  dpevents[ndpevents].events = POLLIN;  break;
    case FDW_WRITE: dpevents[ndpevents].events = POLLOUT; break;
    default:                                              break;
  }

  ndpevents++;
}

static
void
devpoll_del_fd(int fd)
{
  if (ndpevents >= maxdpevents) {
    syslog(LOG_ERR, "too many fds in devpoll_del_fd!");
    return;
  }

  dpevents[ndpevents].fd     = fd;
  dpevents[ndpevents].events = POLLREMOVE;

  ndpevents++;
}

static
int
devpoll_watch(long tmout)
{
  int           i = 0;
  int           r = 0;
  struct dvpoll dvp;

  r = sizeof(struct pollfd) * ndpevents;

  if (r > 0 && write(dp, dpevents, r) != r) {
    return -1;
  }

  ndpevents      = 0;
  dvp.dp_fds     = dprevents;
  dvp.dp_nfds    = nfiles;
  dvp.dp_timeout = (int)tmout;

  if ((r = ioctl(dp, DP_POLL, &dvp)) == -1) {
    return -1;
  }

  for (i = 0; i < r; ++i) {
    dp_rfdidx[dprevents[i].fd] = i;
  }

  return r;
}

static
int
devpoll_check_fd(int fd)
{
  int ridx = dp_rfdidx[fd];

  if (ridx < 0 || ridx >= nfiles) {
    syslog(LOG_ERR, "bad ridx (%d) in devpoll_check_fd!", ridx);
    return 0;
  }

  if (ridx >= nreturned) {
    return 0;
  }

  if (dprevents[ridx].fd != fd) {
    return 0;
  }

  if (dprevents[ridx].revents & POLLERR) {
    return 0;
  }

  switch (fd_rw[fd]) {
    case FDW_READ:
      return dprevents[ridx].events & (POLLIN | POLLHUP | POLLNVAL);

    case FDW_WRITE:
      return dprevents[ridx].events & (POLLOUT | POLLHUP | POLLNVAL);

    default:
      return 0;
  }
}

static
int
devpoll_get_fd(int ridx)
{
  if (ridx < 0 || ridx >= nfiles) {
    syslog(LOG_ERR, "bad ridx (%d) in devpoll_get_fd!", ridx);
    return -1;
  }

  return dprevents[ridx].fd;
}

/* }}} */
/* ========================================================================== */
#elif defined(HAVE_POLL)
/* ========================================================================== */
/* {{{ `poll` implementation: */

static struct pollfd *pollfds;
static int            npoll_fds;
static int           *poll_fdidx;
static int           *poll_rfdidx;

static
int
poll_init(int nf)
{
  int i = 0;

  pollfds     = xmalloc(sizeof(struct pollfd) * nf);
  poll_fdidx  = xmalloc(sizeof(int) * nf);
  poll_rfdidx = xmalloc(sizeof(int) * nf);

  for (i = 0; i < nf; ++i) {
    pollfds[i].fd = poll_fdidx[i] = -1;
  }

  return 0;
}

static
void
poll_add_fd(int fd, int rw)
{
  if (npoll_fds >= nfiles) {
    syslog(LOG_ERR, "too many fds in poll_add_fd!");
    return;
  }

  pollfds[npoll_fds].fd = fd;

  switch (rw) {
    case FDW_READ:  pollfds[npoll_fds].events = POLLIN;  break;
    case FDW_WRITE: pollfds[npoll_fds].events = POLLOUT; break;
    default:                                             break;
  }

  poll_fdidx[fd] = npoll_fds;
  npoll_fds++;
}

static
void
poll_del_fd(int fd)
{
  int idx = poll_fdidx[fd];

  if (idx < 0 || idx >= nfiles) {
    syslog(LOG_ERR, "bad idx (%d) in poll_del_fd!", idx);
    return;
  }

  npoll_fds--;
  pollfds[idx]                = pollfds[npoll_fds];
  poll_fdidx[pollfds[idx].fd] = idx;
  pollfds[npoll_fds].fd       = -1;
  poll_fdidx[fd]              = -1;
}

static
int
poll_watch(long tmout)
{
  int r    = 0;
  int ridx = 0;
  int i    = 0;

  if ((r = poll(pollfds, npoll_fds, (int)tmout)) <= 0) {
    return r;
  }

  for (i = 0; i < npoll_fds; ++i) {
    if (pollfds[i].revents & (POLLIN | POLLOUT | POLLERR | POLLHUP | POLLNVAL))
    {
      poll_rfdidx[ridx++] = pollfds[i].fd;
      if (ridx == r) {
        break;
      }
    }
  }

  return ridx;
}

static
int
poll_check_fd(int fd)
{
  int fdidx = poll_fdidx[fd];

  if (fdidx < 0 || fdidx >= nfiles) {
    syslog(LOG_ERR, "bad fdidx (%d) in poll_check_fd!", fdidx);
    return 0;
  }

  if (pollfds[fdidx].revents & POLLERR) {
    return 0;
  }

  switch (fd_rw[fd]) {
    case FDW_READ:
      return pollfds[fdidx].revents & (POLLIN | POLLHUP | POLLNVAL);

    case FDW_WRITE:
      return pollfds[fdidx].revents & (POLLOUT | POLLHUP | POLLNVAL);

    default:
      return 0;
  }
}

static
int
poll_get_fd(int ridx)
{
  if (ridx < 0 || ridx >= nfiles) {
    syslog(LOG_ERR, "bad ridx (%d) in poll_get_fd!", ridx);
    return -1;
  }

  return poll_rfdidx[ridx];
}

/* }}} */
/* ========================================================================== */
#elif defined(HAVE_SELECT)
/* ========================================================================== */
/* {{{ `select` implementation: */

static fd_set  master_rfdset;
static fd_set  master_wfdset;
static fd_set  working_rfdset;
static fd_set  working_wfdset;
static int    *select_fds;
static int    *select_fdidx;
static int    *select_rfdidx;
static int     nselect_fds;
static int     maxfd;
static int     maxfd_changed;

static
int
select_init(int nf)
{
  int i = 0;

  FD_ZERO(&master_rfdset);
  FD_ZERO(&master_wfdset);

  select_fds    = xmalloc(sizeof(int) * nf);
  select_fdidx  = xmalloc(sizeof(int) * nf);
  select_rfdidx = xmalloc(sizeof(int) * nf);
  nselect_fds   = 0;
  maxfd         = -1;
  maxfd_changed = 0;

  for (i = 0; i < nf; ++i) {
    select_fds[i] = select_fdidx[i] = -1;
  }

  return 0;
}

static
void
select_add_fd(int fd, int rw)
{
  if (nselect_fds >= nfiles) {
    syslog(LOG_ERR, "too many fds in select_add_fd!");
    return;
  }

  select_fds[nselect_fds] = fd;

  switch (rw) {
    case FDW_READ:  FD_SET(fd, &master_rfdset); break;
    case FDW_WRITE: FD_SET(fd, &master_wfdset); break;
    default:                                    break;
  }

  if (fd > maxfd) {
    maxfd = fd;
  }

  select_fdidx[fd] = nselect_fds;
  nselect_fds++;
}

static
void
select_del_fd(int fd)
{
  int idx = select_fdidx[fd];

  if (idx < 0 || idx >= nfiles) {
    syslog(LOG_ERR, "bad idx (%d) in select_del_fd!", idx);
    return;
  }

  nselect_fds--;
  select_fds[idx]               = select_fds[nselect_fds];
  select_fdidx[select_fds[idx]] = idx;
  select_fds[nselect_fds]       = -1;
  select_fdidx[fd]              = -1;

  FD_CLR(fd, &master_rfdset);
  FD_CLR(fd, &master_wfdset);

  if (fd >= maxfd) {
    maxfd_changed = 1;
  }
}

static
int
select_get_maxfd(void)
{
  if (maxfd_changed) {
    int i = 0;

    maxfd = -1;

    for (i = 0; i < nselect_fds; ++i) {
      if (select_fds[i] > maxfd) {
        maxfd = select_fds[i];
      }
    }

    maxfd_changed = 0;

  }

  return maxfd;
}

static
int
select_watch(long tmout)
{
  int mfd  = 0;
  int r    = 0;
  int idx  = 0;
  int ridx = 0;

  working_rfdset = master_rfdset;
  working_wfdset = master_wfdset;
  mfd            = select_get_maxfd();

  if (tmout == INFTIM) {
    r = select(mfd + 1,
               &working_rfdset,
               &working_wfdset,
               NULL,
               NULL);
  } else {
    struct timeval timeout;
    timeout.tv_sec  = tmout / 1000L;
    timeout.tv_usec = (tmout % 1000L) * 1000L;

    r = select(mfd + 1,
               &working_rfdset,
               &working_wfdset,
               NULL,
               &timeout);
  }

  if (r <= 0) {
    return r;
  }

  for (idx = 0; idx < nselect_fds; ++idx) {
    if (select_check_fd(select_fds[idx])) {
      select_rfdidx[ridx++] = select_fds[idx];

      if (ridx == r) {
        break;
      }
    }
  }

  return ridx;
}

static
int
select_check_fd(int fd)
{
  switch (fd_rw[fd]) {
    case FDW_READ:  return FD_ISSET(fd, &working_rfdset);
    case FDW_WRITE: return FD_ISSET(fd, &working_wfdset);
    default:        return 0;
  }
}

static
int
select_get_fd(int ridx)
{
  if (ridx < 0 || ridx >= nfiles) {
    syslog(LOG_ERR, "bad ridx (%d) in select_get_fd!", ridx);
    return -1;
  }

  return select_rfdidx[ridx];
}

/* }}} */
/* ========================================================================== */
#endif

/* fdwatch.c ends here. */
