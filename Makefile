# -*- Mode: Makefile -*-
#
# Makefile --- Sysmon makefile.
#
# Copyright (c) 2017 Paul Ward <asmodai@gmail.com>
#
# Author:     Paul Ward <asmodai@gmail.com>
# Maintainer: Paul Ward <asmodai@gmail.com>
# Created:    19 Jun 2017 04:15:46
#
#{{{ License:
#
# Redistribution and use in source and binary forms, with or without
# modification, are  permitted provided that the following conditions are
# met:
#
# Redistributions of source code must retain the above copyright notice,
# this list of conditions and the following disclaimer. 
#
# Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
# IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
# TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
# PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
# OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#}}}
#{{{ Commentary:
#
#}}}

CC=gcc

CFLAGS=-Wall -pedantic -g -DDEBUG

BIN=sysmon

COMMON_SRCS=utils.c     \
	    json.c      \
	    endpoints.c \
	    fdwatch.c   \
	    timers.c    \
	    httpd.c     \
	    main.c

BSD_SRCS=4bsd_utsname.c

POSIX_SRCS=posix_strtod.c \
	   posix_strdup.c

MODULE_SRCS=sm_smver.c \
	    sm_uname.c \
	    sm_info.c  \
	    sm_cpu.c   \
	    sm_all.c

COMMON_OBJS=utils.o     \
	    json.o      \
	    endpoints.o \
	    fdwatch.o   \
	    timers.o    \
	    httpd.o     \
	    main.o

BSD_OBJS=4bsd_utsname.o

POSIX_OBJS=posix_strtod.o \
	   posix_strdup.o

MODULE_OBJS=sm_smver.o \
	    sm_uname.o \
	    sm_info.o  \
	    sm_cpu.o   \
	    sm_all.o

.c.o:
	${CC} -c ${CFLAGS} $*.c

help:
	@echo "Please use one of the following build targets:"
	@echo "	4BSD		POSIX"

4BSD: 4bsd
4bsd: ${BSD_OBJS} ${POSIX_OBJS} ${MODULE_OBJS} ${COMMON_OBJS}
	${CC} ${LDFLAGS} -o ${BIN} \
		${BSD_OBJS}        \
		${POSIX_OBJS}      \
		${MODULE_OBJS}     \
		${COMMON_OBJS}

POSIX: posix
posix: ${MODULE_OBJS} ${COMMON_OBJS}
	${CC} ${LDFLAGS} -o ${BIN} ${MODULE_OBJS} ${COMMON_OBJS}

clean:
	rm -f *.core core *.o ${BIN}

# Makefile ends here.

