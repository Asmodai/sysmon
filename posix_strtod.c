#include <ctype.h>
#include <sys/param.h>
#include <sys/types.h>
#include <errno.h>

#ifndef TRUE
# define TRUE  1
# define FALSE 0
#endif

#ifndef NULL
#define NULL 0
#endif

static int maxExponent = 511;

static double powersOf10[] = {
  10.0,
  100.0,
  1.0e4,
  1.0e8,
  1.0e16,
  1.0e32,
#ifndef BSD
  1.0e64,
  1.0e128,
  1.0e256
#endif
};

double
strtod(const char *string, char **endPtr)
{
  int                  sign     = FALSE;
  int                  expSign  = FALSE;
  double               fraction = 0.0;
  double               dblExp   = 0.0;
  double              *d        = NULL;
  register const char *p        = NULL;
  register int         c        = 0;
  int                  exp      = 0;
  int                  fracExp  = 0;
  int                  mantSize = 0;
  int                  decPtr   = 0;
  const char          *pExp     = NULL;

  /*
   * Strip off leading blanks.
   */
  p = string;
  while (isspace((u_char)*p)) {
    p++;
  }

  if (*p == '-') {
    sign = TRUE;
    p++;
  } else {
    if (*p == '+') {
      p++;
    }

    sign = FALSE;
  }

  /*
   * Count the number of digits in the mantissa.
   */
  decPtr = -1;
  for (mantSize = 0; ; mantSize++) {
    c = *p;
    
    if (!isdigit(c)) {
      if ((c != '.') || (decPtr >= 0)) {
        break;
      }
      
      decPtr = mantSize;
    }
    
    p++;
  }
  
  /*
   * Now suck up the mantissa.
   */
  pExp  = p;
  p    -= mantSize;
  
  if (decPtr < 0) {
    decPtr = mantSize;
  } else {
    mantSize--;
  }
  
  if (mantSize > 18) {
    fracExp  = decPtr - 18;
    mantSize = 18;
  } else {
    fracExp = decPtr - mantSize;
  }

  if (mantSize == 0) {
    fraction = 0.0;
    p        = string;
    goto done;
  } else {
    int frac1 = 0;
    int frac2 = 0;
     
    for (; mantSize > 9; mantSize--) {
      c = *p++;
      
      if (c == '.') {
        c = *p++;
      }

      frac1 = 10 * frac1 + (c - '0');
    }

    for (; mantSize > 0; mantSize--) {
      c = *p++;

      if (c == '.') {
        c = *p++;
      }

      frac2 = 10 * frac2 + (c - '0');
    }

    fraction = (1.e9 * frac1) + frac2;
  }

  /*
   * Skim off the exponent.
   */
  p = pExp;
  if ((*p == 'E') || (*p == 'e')) {
    p++;

    if (*p == '-') {
      expSign = TRUE;
      p++;
    } else {
      if (*p == '+') {
        p++;
      }

      expSign = FALSE;
    }

    if (!isdigit((u_char)*p)) {
      p = pExp;
      goto done;
    }

    while (isdigit((u_char)*p)) {
      exp = exp * 10 + (*p - '0');
      p++;
    }
  }

  if (expSign) {
    exp = fracExp - exp;
  } else {
    exp = fracExp + exp;
  }

  /*
   * Generate a floating-point number that represents the exponent.
   */

  if (exp < 0) {
    expSign = TRUE;
    exp     = -exp;
  } else {
    expSign = FALSE;
  }

  if (exp > maxExponent) {
    exp   = maxExponent;
    errno = ERANGE;
  }

  dblExp = 1.0;

  for (d = powersOf10; exp != 0; exp >>= 1, d++) {
    if (exp & 01) {
      dblExp *= *d;
    }
  }

  if (expSign) {
    fraction /= dblExp;
  } else {
    fraction *= dblExp;
  }

 done:
  if (endPtr != NULL) {
    *endPtr = (char *)p;
  }

  if (sign) {
    return -fraction;
  }

  return fraction;
}
