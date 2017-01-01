/*
 * fdwatch.h --- file descriptor watching.
 *
 * Copyright (c) 2016 Paul Ward <asmodai@gmail.com>
 * Copyright (c) 1999 Jef Poskanzer <jef@mail.acme.com>
 *
 * Author:     Paul Ward <asmodai@gmail.com>
 * Maintainer: Paul Ward <asmodai@gmail.com>
 * Created:    29 Dec 2016 19:12:44
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
 * @file fdwatch.h
 * @author Paul Ward
 * @brief file descriptor watching.
 */

#ifndef _fdwatch_h_
#define _fdwatch_h_

#define FDW_READ  0
#define FDW_WRITE 1

#ifndef INFTIM
# define INFTIM -1
#endif

int   fdwatch_get_nfiles(void);
void  fdwatch_add_fd(int fd, void *client_data, int rw);
void  fdwatch_del_fd(int fd);
int   fdwatch(long timeout_msecs);
int   fdwatch_check_fd(int fd);
void *fdwatch_get_next_client_data(void);
void  fdwatch_logstats(long secs);

#endif /* !_fdwatch_h_ */

/* fdwatch.h ends here. */
