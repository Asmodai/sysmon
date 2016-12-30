/*
 * config.h --- Configuration.
 *
 * Copyright (c) 2016 Paul Ward <asmodai@gmail.com>
 *
 * Author:     Paul Ward <asmodai@gmail.com>
 * Maintainer: Paul Ward <asmodai@gmail.com>
 * Created:    22 Dec 2016 09:21:54
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
 * @file config.h
 * @author Paul Ward
 * @brief Configuration.
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

/* To get at BSD stuff early on. */
#include <sys/param.h>

/*
 * Define this if you have the stdint.h header file.
 */
#define HAVE_STDINT_H

/*
 * Define this if you have the stdbool.h header file.
 */
#define HAVE_STDBOOL_H

/*
 * Define this if you have the sys/utsname.h header file.
 */
#define HAVE_SYS_UTSNAME_H

/*
 * Default port on which we listen.
 */
#define DEFAULT_PORT 7070

/*
 * How many seconds to allow for reading the initial request on a new
 * connection.
 */
#define IDLE_READ_TIMELIMIT 60

/*
 * How many seconds before an idle connection gets closed.
 */
#define IDLE_SEND_TIMELIMIT 300

/*
 * Syslog log facility to use.
 */
#define LOG_FACILITY LOG_DAEMON

/*
 * Default character set.  We use UTF-8 even on systems that do not do
 * Unicode, this is so we don't have to care.
 */
#define DEFAULT_CHARSET "UTF-8"

/*
 * Undefine this if you don't want the server information to be part
 * of the response headers.
 */
#define SHOW_SERVER_VERSION

/*
 * Default Unix user.
 */
#define DEFAULT_USER "nobody"

/*
 * Default Unix group.
 */
#define DEFAULT_GROUP "nobody"

/*
 * We really need a chroot jail for this daemon, so specify a jail
 * path.
 */     
#define DEFAULT_CHROOT_PATH "/tmp"

/*
 * How often to run the 'garbage collector'.
 */
#define OCCASIONAL_TIME 120

/*
 * Time between updates of the throttle table's rolling averages.
 */
#define THROTTLE_TIME 2

/*
 * The listen() backlog queue length.  Be aware that the system's
 * kernel is free to ignore this value and substitute its own.
 */
#define LISTEN_BACKLOG 1024

/*
 * Maximum number of throttle patterns that any single URL can be
 * included in.
 */
#define MAXTHROTTLENUMS 10

/*
 * Number of file descriptors to reserve for uses other than
 * connections.
 */
#define SPARE_FDS 10

/*
 * How many milliseconds to leave a connection open while doing a
 * lingering close.
 */
#define LINGER_TIME 500

/*
 * This is black magic.
 */
#define MIN_WOULDBLOCK_DELAY 100L

#endif  /* !_CONFIG_H_ */

/* config.h ends here. */
