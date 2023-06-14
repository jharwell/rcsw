/**
 * \file stdio.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/stdio/stdio.h"

#include "rcsw/stdio/math.h"
#include "rcsw/stdio/string.h"

/*******************************************************************************
 * API Functions
 ******************************************************************************/
char* stdio_dtoa(double n, bool_t force_exp, char* s) {
  /* handle special cases */
  if (stdio_isnan(n)) {
    stdio_strcpy(s, "nan");
    return s;
  } else if (stdio_isinf(n)) {
    stdio_strcpy(s, "inf");
    return s;
    /* We are in a bootstrap/don't have stdlib--nothing we can do */
    RCSW_WARNING_DISABLE_PUSH();
    RCSW_WARNING_DISABLE_FLOAT_EQUAL()
  } else if (n == 0.0) {
    RCSW_WARNING_DISABLE_POP()
    stdio_strcpy(s, "0");
    return s;
  }

  int digit;
  char* c = s;
  bool_t neg = (n < 0);
  if (neg) {
    n = -n;
  }
  int m = stdio_log10(n); /* calculate magnitude */

  /*
   * Use scientific notation (SN) if:
   * (1) n > 0 and n > 10^14
   * (2) n < 0 and |n| < 10^(9),
   * (3) n > 0 and n < 10^(-9)
   * (4) force_exp was passed.
   */
  bool_t use_exp = (m >= 14 || (neg && m >= 9) || m <= -9 || force_exp);

  /* make 0 < n < 10, saving off the power of 10 that it took to do so */
  int exp = 0;

  if (use_exp) {
    if (m < 0 || ((m == 0) && (n > 0) && (n < 1))) {
      m -= 1;
    }
    n /= stdio_pow10(m);
    exp = m;
    m = 0;
  }

  if (neg) {
    *(c++) = '-';
  }

  /*
   * At this point, m = 0 if -10 < n < 10 or if SN will be used.
   * Convert the base if using SN, full number otherwise. Iteratively get
   * most significant digit, reduce n by digit*10^m.
   */
  while (n > DBL_MIN || m >= 0) {
    double weight = stdio_pow10(m);

    if (weight > 0 && !stdio_isinf(weight)) {
      digit = (int)(stdio_floor(n / weight));
      n -= (digit * weight);
      *(c++) = '0' + (char)digit;
    }
    if (m == 0 && n > 0) {
      *(c++) = '.';
    }
    m--;
  } /* end while() */

  if (use_exp) { /* convert the exponent */
    *(c++) = 'e';
    if (exp >= 0) {
      *(c++) = '+';
    } else {
      *(c++) = '-';
      exp = -exp;
    }

    /*
     * Reset m to 0, but it has a new meaning: the number of exponent
     * digits.
     */
    m = 0;
    if (exp == 0) {
      *(c++) = '0';
      m++;
    } else {
      while (exp > 0) {
        *(c++) = '0' + exp % 10;
        exp /= 10;
        m++;
      }
    }
    /*
     * Back c up by the number of digits translated for the exponent. It now
     * points to the start of the converted exponent string. The digits are
     * in reverse order, so reverse them to get the correct exponent string.
     */
    c -= m;
    stdio_strrev(c, m);
    c += m;
  } /* end if */

  *(c) = '\0';
  return c;
} /* stdio_dtoa() */

size_t stdio_puts(const char* const s) {
  size_t i;
  for (i = 0; i < stdio_strlen(s); i++) {
    stdio_putchar(s[i]);
  } /* for() */
  return i;
} /* stdio_puts() */

int stdio_atoi(const char* s, int base) {
  char c;
  int result = 0;

  while (*s == ' ') {
    ++s; /* advance past any spaces */
  }

  int neg = (*s == '-') ? 1 : 0;
  int pos = (*s == '+') ? 1 : 0;

  if (neg || pos) {
    s++; /* advance pass the '-'/'+' */
  }

  if (base == 16) {
    s += 2; /* advance past the '0x' */
  }

  while (*s != '\0') {
    c = (char)stdio_toupper(*s);
    if ((c >= '0') && (c <= '9')) {
      result = (result * base) + (c - '0');
    } else if ((c >= 'A') && (c <= 'F') && (base == 16)) {
      result = (result * base) + (c + 10 - 'A');
    } else {
      break; /* unknown char--bail */
    }
    ++s;
  } /* while() */

  return neg ? -result : result;
} /* stdio_atoi() */

char* stdio_itoad(int n, char* s) {
  int i = 0;

  if (n == 0) {
    s[i++] = '0';
  } else if (n < 0) {
    s[i++] = '-';
    n = -n;
  } else {
    s[i++] = '+';
  }
  while (n > 0) {
    s[i++] = '0' + n % 10;
    n /= 10;
  }

  s[i] = '\0';
  stdio_strrev(s + 1, i - 1);
  return s;
} /* stdio_itoad() */

char* stdio_itoax(int i, char* s, bool_t add_0x) {
  size_t n;
  size_t n_digits;

  if (i < 0x10) {
    n_digits = 1;
  } else if (i < 0x100) {
    n_digits = 2;
  } else if (i < 0x1000) {
    n_digits = 3;
  } else if (i < 0x10000) {
    n_digits = 4;
  } else if (i < 0x100000) {
    n_digits = 5;
  } else if (i < 0x1000000) {
    n_digits = 6;
  } else if (i < 0x10000000) {
    n_digits = 7;
  } else {
    n_digits = 8;
  }

  if (add_0x) {
    *s++ = '0';
    *s++ = 'x';
  }

  s += n_digits;
  *s = '\0';
  for (n = n_digits; n != 0; --n) {
    *--s = "0123456789abcdef"[i & 0x0F];
    i >>= 4;
  }
  if (add_0x) {
    return s - 2;
  } else {
    return s;
  }
} /* stdio_itoax() */

END_C_DECLS
