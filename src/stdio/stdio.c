/**
 * \file
 *
 * \copyright 2017 John Harwell
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/stdio/stdio.h"

#include <errno.h>

#include "rcsw/core/fpc.h"
#include "rcsw/stdio/string.h"

/*******************************************************************************
 * Callback Functions
 ******************************************************************************/
/*
 * This is the name of the putchar function that the printf() library expects,
 * so we shim it to stdio_putchar().
 */
/* NOLINTBEGIN(readability-identifier-naming) */
void putchar_(char c);
void putchar_(char c) { stdio_putchar(c); }
/* NOLINTEND(readability-identifier-naming) */

/*******************************************************************************
 * Public API
 ******************************************************************************/
int stdio_putchar(int c) { return RCSW_CONFIG_STDIO_PUTCHAR(c); }

int stdio_getchar(void) { return RCSW_CONFIG_STDIO_GETCHAR(); }

size_t stdio_puts(const char* const s) {
  size_t i;
  for (i = 0; i < stdio_strlen(s); i++) {
    stdio_putchar(s[i]);
  } /* for() */
  return i;
} /* stdio_puts() */

int stdio_atoi(const char* s, int base) {
  RCSW_FPC_NV(0, NULL != s, base >= 2, base <= 16);
  /* Accumulate unsigned: INT_MIN parses, and overflow wraps rather than UB */
  unsigned result = 0;

  while (*s == ' ') {
    ++s; /* advance past any spaces */
  }

  int neg = (*s == '-') ? 1 : 0;
  int pos = (*s == '+') ? 1 : 0;

  if (neg || pos) {
    s++; /* advance pass the '-'/'+' */
  }

  /* The 0x prefix is optional for base 16 */
  if (base == 16 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
    s += 2;
  }

  /* Stop at the first character that is not a digit in this base */
  for (; *s != '\0'; ++s) {
    char c     = (char)stdio_toupper(*s);
    int  digit = -1;
    if ((c >= '0') && (c <= '9')) {
      digit = c - '0';
    } else if ((c >= 'A') && (c <= 'F')) {
      digit = c - 'A' + 10;
    }
    if (digit < 0 || digit >= base) {
      break;
    }
    result = (result * (unsigned)base) + (unsigned)digit;
  } /* for(s..) */

  return (int)(neg ? 0U - result : result);
} /* stdio_atoi() */

char* stdio_itoad(int32_t n, char* s, size_t len) {
  RCSW_FPC_NV(NULL, NULL != s);

  /* Work with the magnitude as unsigned: -INT32_MIN does not fit in int32_t */
  uint32_t mag = (n < 0) ? (0U - (uint32_t)n) : (uint32_t)n;

  size_t n_digits = 1;
  for (uint32_t tmp = mag; tmp >= 10; tmp /= 10) {
    ++n_digits;
  }
  /* sign (except for 0) + digits + NUL */
  size_t needed = n_digits + ((n == 0) ? 0U : 1U) + 1;
  if (len < needed) {
    errno = ENOSPC;
    return NULL;
  }

  size_t i = 0;
  if (n < 0) {
    s[i++] = '-';
  } else if (n > 0) {
    s[i++] = '+';
  }
  s[i + n_digits] = '\0';
  for (size_t k = n_digits; k != 0; --k) {
    s[i + k - 1] = (char)('0' + (int)(mag % 10));
    mag /= 10;
  }
  return s;
} /* stdio_itoad() */

char* stdio_itoax(uint32_t i, char* s, size_t len, bool_t add_0x) {
  RCSW_FPC_NV(NULL, NULL != s);

  size_t n_digits = 1;
  for (uint32_t tmp = i; tmp >= 0x10; tmp >>= 4) {
    ++n_digits;
  }
  size_t prefix = add_0x ? 2U : 0U;
  if (len < prefix + n_digits + 1) {
    errno = ENOSPC;
    return NULL;
  }

  if (add_0x) {
    s[0] = '0';
    s[1] = 'x';
  }
  s[prefix + n_digits] = '\0';
  for (size_t k = n_digits; k != 0; --k) {
    s[prefix + k - 1] = "0123456789abcdef"[i & 0x0F];
    i >>= 4;
  }
  return s;
} /* stdio_itoax() */

END_C_DECLS
