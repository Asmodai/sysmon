/*
 * utsname.c --- 4BSD `uname` implementation.
 *
 * Copyright (c) 2016 Paul Ward <asmodai@gmail.com>
 *
 * Author:     Paul Ward <asmodai@gmail.com>
 * Maintainer: Paul Ward <asmodai@gmail.com>
 * Created:    22 Dec 2016 09:28:03
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
 * @file utsname.c
 * @author Paul Ward
 * @brief 4BSD `uname` implementation.
 */

#include "config.h"

#if !PLATFORM_EQ(PLATFORM_BSD)
# error "You don't want this."
#else

# include <stdio.h>
# include <ctype.h>
# include <nlist.h>
# include <memory.h>

# include <sys/param.h>
# include <sys/file.h>

# define UTSNAME_IMPLEMENTATION 1
# include "4bsd/utsname.h"
# undef UTSNAME_IMPLEMENTATION

struct nlist knl[] = {
  { "_version" },
  { 0 }
};

int
uname(struct utsname *name)
{
  size_t  idx  = 0;
  int     kmem = -1;
  char   *p    = NULL;

  extern char *index();
  extern       gethostname();
  extern       fprintf();
  extern       nlist();
  extern       open();
  extern       lseek();
  extern       read();

  memset(name->sysname,  0, _SYS_NAMELEN);
  memset(name->nodename, 0, _SYS_NAMELEN);
  memset(name->release,  0, _SYS_NAMELEN);
  memset(name->version,  0, _SYS_NAMELEN);
  memset(name->machine,  0, _SYS_NAMELEN);
 
  if (gethostname(name->nodename, _SYS_NAMELEN)) {
    fprintf(stderr, "Could not get hostname.\n");
    return -1;
  }

  nlist("/vmunix", knl);
  if (knl[0].n_type == 0) {
    fprintf(stderr, "Could not get kernel namelist.\n");
    return -1;
  }

  if ((kmem = open("/dev/kmem", 0)) < 0) {
    fprintf(stderr, "Could not open kernel memory.\n");
    return -1;
  }

  lseek(kmem, (long)knl[0].n_value, L_SET);
  read(kmem, name->version, _SYS_NAMELEN);

  while ((p = index(name->version, '\n')) != NULL) {
    *p = ' ';
  }
  
  for (idx = 0; idx < _SYS_NAMELEN; idx++) {
    if (isdigit(name->version[idx])   &&
        name->version[idx + 1] == '.' &&
        isdigit(name->version[idx] + 2))
    {
      sprintf(name->release,
              "%c.%c",
              name->version[idx],
              name->version[idx + 2]);
      break;
    }
  }
  
  sprintf(name->machine, "%s", MACHINE);
  sprintf(name->sysname,
          "%s", 
# if PLATFORM_EQ(PLATFORM_BSD)
          "BSD"
# else 
          "UNKNOWN"
# endif
          );

  return 0;
}

#endif  /* BSD */

/* utsname.c ends here. */
