/**
 * \file sstdio.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/stdio/sstdio.h"
#include "rcsw/stdio/sstring.h"

#include <math.h>

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
#define FLOATING_POINT_PRECISION 0.000000000000001 /* 15 digits */

/*******************************************************************************
 * Macros
 ******************************************************************************/
#define DISPLAY_FILL_CHARS(_char, num)    \
  for (int _i = 0; _i < (int)num; _i++) { \
    sstdio_putchar(_char);                \
  }

/*******************************************************************************
 * Forward Declarations
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Get number of digits to display for int in printf().
 *
 * Get the number of digits to display based on the current position within the
 * format block. This is a helper function to help reduce code duplication.
 *
 * \param fmt_buf Scratch area.
 * \param fmt_block The format string.
 * \param pos Position within format string.
 *
 * \return The number of digits to display.
 */
static int decimal_digits_get(const char* fmt_buf,
                              const char* fmt_block,
                              size_t pos);

/**
 * \brief Round a floating point string to a specified number of digits
 *
 * This function takes and modifies digits AFTER the number of requested decimal
 * places so that numbers appear rounded. This is a helper function to help
 * reduce code duplication.
 *
 * \param s The converted string
 * \param n_digits The # of decimal places to keep.
 */
static void float_arg_round(char* s, size_t n_digits);

/*******************************************************************************
 * API Functions
 ******************************************************************************/
RCSW_WARNING_DISABLE_PUSH()
RCSW_WARNING_DISABLE_VLA()
int sstdio_printf(const char* fmt, ...) {
  va_list arg;
  int rval;
  /* 2* is for a safety margin when doing strrep() */
  char proc_str[2 * sstring_strlen(fmt)];

  va_start(arg, fmt);
  /* Needed to get things to display correctly in serial terminal */
  sstring_strrep(fmt, "\n", "\r\n", proc_str);

  rval = sstdio_vprintf(proc_str, arg);
  va_end(arg);
  return rval;
} /* sstdio_printf() */
RCSW_WARNING_DISABLE_POP()

int sstdio_vprintf(const char* fmt, va_list argp) {
  const char* p;
  const char* s;
  int i, n_digits;
  int c, val;
  double dval;

  /* will be either ' ' or 0, depending on format block */
  char fill_char;
  int show_sign = 0;    /* flag to always show sign on integers */
  int left_justify = 0; /* flag to left justify integer formatting */
  char fmt_buf[256];
  char fmt_chars[24] = {
      'd',      /* decimal integers */
      'x', 'X', /* hexadecimal integers (always size_t) */
      's',      /* strings */
      'c',      /* chars */
      'p',      /* pointers */

      'f', /* floating point */
      'e', 'E',

      '.', '+', '-',                              /* format modifiers */
      '0', '1', '2', '3', '4', '5', '6', '7', '8' /* numerical specifiers */
  };

  /*
   * A format block is the format char(s) that are applied to a single
   * argument from the variable argument list
   */
  char fmt_block[16];

  for (p = fmt, c = 0; *p != '\0'; p++, c++) {
    if (*p != '%') { /* just a regular char */
      sstdio_putchar(*p);
      continue;
    }

    s = ++p; /* move past % to first format specifier in block */

    /*
     * Catch the %% case where the user wants to print a literal %. I can't
     * include % in fmt_chars, because then stuff like %s%c%s would be
     * considered 1 fmt_block, instead of 3.
     */
    if (*s == '%') {
      sstdio_putchar('%');
      continue;
    }
    /* get all format specifiers/modifiers */
    for (i = 0; sstring_strchr(fmt_chars, *s); i++) {
      fmt_block[i] = *s++;
    }

    /* NULL terminate fmt_block so string functions work as expected */
    fmt_block[i] = '\0';

    p += i - 1; /* move p forward by the # of chars in the fmt_block */
    size_t val2;

    /* process format specifiers/modifiers */
    for (i = 0; i < (int)sstring_strlen(fmt_block); i++) {
      fill_char = ' ';
      switch (fmt_block[i]) {
        case '+':
          show_sign = 1;
          break;
        case '-':
          left_justify = 1;
          break;
        case 'c':
          sstdio_putchar((char)va_arg(argp, int));
          break;

        case 'u':
          val2 = va_arg(argp, size_t);
          sstdio_itoad((int)val2, fmt_buf);
          sstdio_puts(fmt_buf);
          break;
        case 'f':
        case 'e':
        case 'E':

          dval = va_arg(argp, double);
          char exp_char = fmt_block[i];
          int j, k;

          if (exp_char == 'e' || exp_char == 'E') {
            sstdio_dtoa(dval, 1, fmt_buf);
          } else {
            sstdio_dtoa(dval, 0, fmt_buf);
          }

          /* get number of decimals to display (max of 8) */
          if ((sstring_strlen(fmt_block) > 1 && fmt_block[i - 2] == '.') &&
              (sstring_isdigit(fmt_block[i - 1])) && fmt_block[i - 1] <= '8') {
            n_digits = sstdio_atoi(&fmt_block[i - 1], 10);
          } else {
            n_digits = 6;
          }

          s = fmt_buf;

          if (sstring_strchr(s, '.')) {
            /* diplay arg mantissa */

            j = 0; /* count of mantissa chars */

            /* display all chars up to decimal point */
            while (*s != '.') {
              sstdio_putchar(*s++);
              j++;
            }

            size_t tmp_len = sstring_strlen(s);

            /* display '.' */
            sstdio_putchar(*s++);

            /*
             * \todo This is necessary because this function and/or some of its
             * subfunctions are not quite const correct. They should be made
             * that way at some point...
             */
            RCSW_WARNING_DISABLE_PUSH()
            RCSW_WARNING_DISABLE_QUAL()
            /* round arg decimal portion as needed */
            float_arg_round(s, n_digits);
            RCSW_WARNING_DISABLE_POP()

            /* display arg decimals */

            j = 0; /* count of decimal chars */

            /*
           * Display all decimals up to exponent, or up to the number
           * of requested digits, whichever comes first
           */
            while (*s != exp_char && j < (int)tmp_len) {
              sstdio_putchar(*s++);
              j++;
              if (j == n_digits) {
                break;
              }
            }

            /* if there were not enough, 0 fill */
            for (k = 0; k < n_digits - j; k++) {
              sstdio_putchar('0');
            }
          } else { /* arg did not have a '.' */
            /*
           * Display all characters up to exponent or the end of the
           * arg, whichever comes first
           */
            j = 0; /* count of mantissa chars */
            size_t tmp_len = sstring_strlen(s);
            while (*s != 'e' && j < (int)tmp_len) {
              sstdio_putchar(*s++);
              j++;
            }
            /* display a fill decimal region */
            sstdio_putchar('.');
            DISPLAY_FILL_CHARS('0', n_digits);
          }

          /* display the exponent for arg, if it contains one */
          if (sstring_strchr(s, 'e')) {
            /*
           * Advance to the start of the exponent, possibly skipping
           * digits
           */
            while (*s != 'e') {
              s++;
            }
            /* display the 'e'/'E' and advance past it */
            sstdio_putchar((exp_char == 'E') ? 'E' : 'e');
            s++;

            /* display sign of exponent */
            sstdio_putchar(*s++);

            /* if the exponent has only a single digit, add a fill 0 */
            if (sstring_strlen(s) < 2) {
              sstdio_putchar('0');
            }
            /* display the rest of the exponent */
            sstdio_puts(s);
          }
          break;
        case 'd':
          val = va_arg(argp, int);
          sstdio_itoad(val, fmt_buf);

          /* Get fill char */
          if (i >= 2 && fmt_block[i - 2] == '0') {
            fill_char = '0';
          }
          /* needed because fmt_buf is an immutable array -_- */
          s = fmt_buf;

          /*
         * Advance past the sign returned by itoa() if not displaying the sign
         * char (except if the fmt_buf is the single digit number 0)
         */
          if (fill_char == ' ') {
            if (!show_sign && val > 0) {
              s++;
            }
          }
          /*
         * Display the sign char appropriately for zero-filled numbers. The sign
         * char is counted as one of the number of requested characters, if a
         * certain number of digits is requested.
         */
          else if (fill_char == '0') {
            if (val < 0) {
              sstdio_putchar('-');
              n_digits--;
            } else if (show_sign) {
              sstdio_putchar('+');
              n_digits--;
            }
            s++;
          }
          /* get number of digits to display */
          n_digits = (size_t)decimal_digits_get(s, fmt_block, i);

          int tmp_len = sstring_strlen(s);

          /* only allow left justification if space-filling */
          if (fill_char != ' ') {
            left_justify = 0;
          }

          /*
         * Display fill chars if necessary; if strlen(fmt_buf) > n_digits,
         * ignore the request to print only a certain number of digits.
         */
          if (tmp_len < n_digits && !left_justify) {
            DISPLAY_FILL_CHARS(fill_char, (int)n_digits - (int)tmp_len);
          }
          /* display arg */
          while (*s) {
            sstdio_putchar(*s++);
          }
          if (left_justify) {
            DISPLAY_FILL_CHARS(fill_char, (int)n_digits - (int)tmp_len);
          }
          break;

        case 's':
          sstdio_puts(va_arg(argp, char*));
          break;
        case 'X':
        case 'x':
          /* get and convert arg */
          val2 = va_arg(argp, size_t);
          sstdio_itoax((int)val2, fmt_buf);

          /* get number of digits to display */
          n_digits = (size_t)decimal_digits_get(fmt_buf, fmt_block, i);

          /*
         * Display fill chars if necessary; if strlen(fmt_buf) >
         * n_digits, ignore the request to print only a certain number
         * of digits
         */
          if (sstring_strlen(fmt_buf) < (size_t)n_digits) {
            DISPLAY_FILL_CHARS(fill_char,
                               (int)n_digits - (int)sstring_strlen(s));
          }

          /* display arg */
          s = fmt_buf;
          while (*s) {
            if (fmt_block[i] == 'X') {
              sstdio_putchar((char)sstring_toupper(*s++));
            } else {
              sstdio_putchar(*s++);
            }
          }
          break;
        case 'p':
          val2 = va_arg(argp, size_t);
          sstdio_itoax((int)val2, fmt_buf);
          sstdio_puts("0x");
          sstdio_puts(fmt_buf);
          break;
        default:
          break;
      } /* end switch */
    }   /* end for() */
  }     /* end for() */

  return c;
} /* sstdio_vprintf() */

char* sstdio_dtoa(double n, bool_t force_exp, char* s) {
  /* handle special cases */
  if (isnan(n)) {
    sstring_strcpy(s, "nan");
    return s;
  } else if (isinf(n)) {
    sstring_strcpy(s, "inf");
    return s;
/* We are in a bootstrap/don't have stdlib--nothing we can do */
    RCSW_WARNING_DISABLE_PUSH();
    RCSW_WARNING_DISABLE_FLOAT_EQUAL()
  } else if (n == 0.0) {
    RCSW_WARNING_DISABLE_POP()
    sstring_strcpy(s, "0");
    return s;
  }

  int digit, m, exp;
  char* c = s;
  int neg = (n < 0);
  exp = 0;
  if (neg) {
    n = -n;
  }
  RCSW_WARNING_DISABLE_PUSH();
  RCSW_WARNING_DISABLE_FUNC_CAST();
  m = (int)log10(n); /* calculate magnitude */
  RCSW_WARNING_DISABLE_POP();
  /*
   * Use scientific notation (SN) if:
   * (1) n > 0 and n > 10^14
   * (2) n < 0 and |n| < 10^(9),
   * (3) n > 0 and n < 10^(-9)
   * (4) force_exp was passed.
   */
  int useExp = (m >= 14 || (neg && m >= 9) || m <= -9 || force_exp);
  /* make 0 < n < 10, saving off the power of 10 that it took to do so */
  if (useExp) {
    if (m < 0 || ((m == 0) && (n > 0) && (n < 1))) {
      m -= 1;
    }
    n /= pow(10.0, m);
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
  while (n > FLOATING_POINT_PRECISION || m >= 0) {
    double weight = pow(10.0, m);
    if (weight > 0 && !isinf(weight)) {
      RCSW_WARNING_DISABLE_PUSH()
      RCSW_WARNING_DISABLE_FUNC_CAST()
      digit = (int)floor(n / weight);
      RCSW_WARNING_DISABLE_POP()

      n -= (digit * weight);
      *(c++) = '0' + (char)digit;
    }
    if (m == 0 && n > 0) {
      *(c++) = '.';
    }
    m--;
  } /* end while() */

  if (useExp) { /* convert the exponent */
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
    sstring_strrev(c, m);
    c += m;
  } /* end if */

  *(c) = '\0';
  return c;
} /* sstdio_dtoa() */

size_t sstdio_puts(const char* const s) {
  size_t i;
  for (i = 0; i < sstring_strlen(s); i++) {
    sstdio_putchar(s[i]);
  } /* for() */
  return i;
} /* sstdio_puts() */

int sstdio_atoi(const char* s, int base) {
  char c;
  int result = 0;
  int neg = (*s == '-') ? 1 : 0;

  if (neg) {
    s++;
  }
  if (base == 16) {
    s += 2; /* advance past the 0x */
  }

  while (1) {
    c = (char)sstring_toupper(*s++);
    if ((c >= '0') && (c <= '9')) {
      result = (result * base) + (c - '0');
    }

    else if ((c >= 'A') && (c <= 'F') && (base == 16)) {
      result = (result * base) + (c + 10 - 'A');
    } else {
      break;
    }
  }

  return neg ? -result : result;
} /* sstdio_atoi() */

char* sstdio_itoad(int n, char* s) {
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
  sstring_strrev(s + 1, i - 1);
  return s;
} /* sstdio_itoad() */

char* sstdio_itoax(int i, char* s) {
  size_t n;
  size_t n_digits;
  int j = i;

  if (j < 0x10) {
    n_digits = 1;
  } else if (j < 0x100) {
    n_digits = 2;
  } else if (j < 0x1000) {
    n_digits = 3;
  } else if (j < 0x10000) {
    n_digits = 4;
  } else if (j < 0x100000) {
    n_digits = 5;
  } else if (j < 0x1000000) {
    n_digits = 6;
  } else if (j < 0x10000000) {
    n_digits = 7;
  } else {
    n_digits = 8;
  }

  s += n_digits;
  *s = '\0';
  for (n = n_digits; n != 0; --n) {
    *--s = "0123456789abcdef"[i & 0x0F];
    i >>= 4;
  }
  return s;
} /* sstdio_itoax() */

/*******************************************************************************
 * Static Functions
 ******************************************************************************/
static int decimal_digits_get(const char* const fmt_buf,
                              const char* const fmt_block,
                              size_t pos) {
  /* get number of digits to display */
  if (sstring_strlen(fmt_block) > 1 && sstring_isdigit(fmt_block[pos - 1])) {
    return sstdio_atoi(&fmt_block[pos - 1], 10);
  } else {
    return (int)sstring_strlen(fmt_buf);
  }
} /* decimal_digits_get() */

static void float_arg_round(char* s, size_t n_digits) {
  size_t i;
  char* tmp;

  i = 0;

  if (sstring_strchr(s, 'e')) {
    /* get number of chars used by the exponent */
    tmp = s + sstring_strlen(s);
    while (*tmp-- != 'e') {
      i++;
    }
  }

  /* i holds the index of the char before the 'e' char */
  i = sstring_strlen(s) - i - 1;

  /*
   * Round and propagate though a series of 9's up to the digit AFTER
   * the last requested digit.
   */
  while (s[i] == '9' && s[i - 1] == '9') {
    s[i--] = '0';
    if (s[i] != '9' || s[i - 1] != '9') {
      s[i--] = '0';
      s[i]++;
      break;
    }
  }

  /*
   * Round the last of the requested digits, possibly propagating a carry, if
   * that digit was a 9.
   */
  if (sstring_isdigit(s[n_digits]) && s[n_digits] >= '5') {
    if (s[n_digits - 1] != '9') {
      s[n_digits - 1]++;
    } else {
      i = n_digits - 1;

      /* propagate a carry through a sequence of 9's */
      while (s[i] == '9' && s[i - 1] == '9') {
        s[i--] = '0';
      }
      s[i--] = '0';
      s[i]++;
    }
  }
} /* float_arg_round() */

END_C_DECLS
