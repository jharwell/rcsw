/**
 * \file printf_internal.c
 *
 * \author (c) Eyal Rozenberg <eyalroz1@gmx.com>
 *             2021-2022, Haifa, Palestine/Israel
 * \author (c) Marco Paland (info@paland.com)
 *             2014-2019, PALANDesign Hannover, Germany
 *
 * SPDX-License Identifier:
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/stdio/printf_internal.h"

/*******************************************************************************
 * API Functions
 ******************************************************************************/
void out_reversed(struct printf_output_gadget* output,
              const char* buf,
              printf_size_t len,
              printf_size_t width,
              printf_flags_t flags) {
  const printf_size_t start_pos = output->pos;

  // pad spaces up to given width
  if (!(flags & FLAGS_LEFT) && !(flags & FLAGS_ZEROPAD)) {
    for (printf_size_t i = len; i < width; i++) {
      gadget_putchar(output, ' ');
    }
  }

  // reverse string
  while (len) {
    gadget_putchar(output, buf[--len]);
  }

  // append pad spaces up to given width
  if (flags & FLAGS_LEFT) {
    while (output->pos - start_pos < width) {
      gadget_putchar(output, ' ');
    }
  }
}

void print_integer(struct printf_output_gadget* output,
                   printf_unsigned_value_t value,
                   bool_t negative,
                   numeric_base_t base,
                   printf_size_t precision,
                   printf_size_t width,
                   printf_flags_t flags) {
  char buf[RCSW_STDIO_PRINTF_BUFFER_SIZE];
  printf_size_t len = 0U;

  if (!value) {
    if (!(flags & FLAGS_PRECISION)) {
      buf[len++] = '0';
      flags &= ~FLAGS_HASH;
      // We drop this flag this since either the alternative and regular modes
      // of the specifier don't differ on 0 values, or (in the case of octal)
      // we've already provided the special handling for this mode.
    } else if (base == BASE_HEX) {
      flags &= ~FLAGS_HASH;
      // We drop this flag this since either the alternative and regular modes
      // of the specifier don't differ on 0 values
    }
  } else {
    do {
      const char digit = (char)(value % base);
      buf[len++] =
          (char)(digit < 10 ? '0' + digit
                            : (flags & FLAGS_UPPERCASE ? 'A' : 'a') + digit - 10);
      value /= base;
    } while (value && (len < RCSW_STDIO_PRINTF_BUFFER_SIZE));
  }

  print_integer_finalization(
      output, buf, len, negative, base, precision, width, flags);
}

void print_integer_finalization(struct printf_output_gadget* output,
                                char* buf,
                                printf_size_t len,
                                bool_t negative,
                                numeric_base_t base,
                                printf_size_t precision,
                                printf_size_t width,
                                printf_flags_t flags) {
  printf_size_t unpadded_len = len;

  // pad with leading zeros
  {
    if (!(flags & FLAGS_LEFT)) {
      if (width && (flags & FLAGS_ZEROPAD) &&
          (negative || (flags & (FLAGS_PLUS | FLAGS_SPACE)))) {
        width--;
      }
      while ((flags & FLAGS_ZEROPAD) && (len < width) &&
             (len < RCSW_STDIO_PRINTF_BUFFER_SIZE)) {
        buf[len++] = '0';
      }
    }

    while ((len < precision) && (len < RCSW_STDIO_PRINTF_BUFFER_SIZE)) {
      buf[len++] = '0';
    }

    if (base == BASE_OCTAL && (len > unpadded_len)) {
      // Since we've written some zeros, we've satisfied the alternative format
      // leading space requirement
      flags &= ~FLAGS_HASH;
    }
  }

  // handle hash
  if (flags & (FLAGS_HASH | FLAGS_POINTER)) {
    if (!(flags & FLAGS_PRECISION) && len &&
        ((len == precision) || (len == width))) {
      // Let's take back some padding digits to fit in what will eventually
      // be the format-specific prefix
      if (unpadded_len < len) {
        len--; // This should suffice for BASE_OCTAL
      }
      if (len && (base == BASE_HEX || base == BASE_BINARY) &&
          (unpadded_len < len)) {
        len--; // ... and an extra one for 0x or 0b
      }
    }
    if ((base == BASE_HEX) && !(flags & FLAGS_UPPERCASE) &&
        (len < RCSW_STDIO_PRINTF_BUFFER_SIZE)) {
      buf[len++] = 'x';
    } else if ((base == BASE_HEX) && (flags & FLAGS_UPPERCASE) &&
               (len < RCSW_STDIO_PRINTF_BUFFER_SIZE)) {
      buf[len++] = 'X';
    } else if ((base == BASE_BINARY) && (len < RCSW_STDIO_PRINTF_BUFFER_SIZE)) {
      buf[len++] = 'b';
    }
    if (len < RCSW_STDIO_PRINTF_BUFFER_SIZE) {
      buf[len++] = '0';
    }
  }

  if (len < RCSW_STDIO_PRINTF_BUFFER_SIZE) {
    if (negative) {
      buf[len++] = '-';
    } else if (flags & FLAGS_PLUS) {
      buf[len++] = '+'; // ignore the space if the '+' exists
    } else if (flags & FLAGS_SPACE) {
      buf[len++] = ' ';
    }
  }

  out_reversed(output, buf, len, width, flags);
}
