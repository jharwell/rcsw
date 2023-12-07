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
int stdio_putchar(int c) {
  return RCSW_CONFIG_STDIO_PUTCHAR(c);
}

int stdio_getchar(void) {
  return RCSW_CONFIG_STDIO_GETCHAR();
}

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

char* stdio_itoad(int32_t n, char* s) {
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

char* stdio_itoax(uint32_t i, char* s, bool_t add_0x) {
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
