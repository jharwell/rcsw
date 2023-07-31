/**
 * \file printf.c
 *
 * \author (c) Eyal Rozenberg <eyalroz1@gmx.com>
 *             2021-2022, Haifa, Palestine/Israel
 * \author (c) Marco Paland (info@paland.com)
 *             2014-2019, PALANDesign Hannover, Germany
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/stdio/printf.h"

#include "rcsw/stdio/printf_internal.h"
#include "rcsw/stdio/string.h"

/*******************************************************************************
 * Misc. Configuration
 ******************************************************************************/
#if RCSW_STDIO_PRINTF_CHECK_NULL
#define ADVANCE_IN_FORMAT_STRING(cptr_) \
  do {                                  \
    (cptr_)++;                          \
    if (!*(cptr_)) {                    \
      return;                           \
    }                                   \
  } while (0)

#else
#define ADVANCE_IN_FORMAT_STRING(cptr_) (cptr_)++
#endif

/*******************************************************************************
 * Private Functions
 ******************************************************************************/
BEGIN_C_DECLS
// internal ASCII string to printf_size_t conversion
static printf_size_t stdio_atou(const char** str) {
  printf_size_t i = 0U;
  while (STDIO_ISDIGIT(**str)) {
    i = i * 10U + (printf_size_t)(*((*str)++) - '0');
  }
  return i;
}

// Advances the format pointer past the flags, and returns the parsed flags
// due to the characters passed
static printf_flags_t parse_flags(const char** format) {
  printf_flags_t flags = 0U;
  do {
    switch (**format) {
      case '0':
        flags |= FLAGS_ZEROPAD;
        (*format)++;
        break;
      case '-':
        flags |= FLAGS_LEFT;
        (*format)++;
        break;
      case '+':
        flags |= FLAGS_PLUS;
        (*format)++;
        break;
      case ' ':
        flags |= FLAGS_SPACE;
        (*format)++;
        break;
      case '#':
        flags |= FLAGS_HASH;
        (*format)++;
        break;
      default:
        return flags;
    }
  } while (true);
}

static inline void format_string_loop(struct printf_output_gadget* output,
                                      const char* format,
                                      va_list args) {
  while (*format) {
    if (*format != '%') {
      // A regular content character
      gadget_putchar(output, *format);
      format++;
      continue;
    }
    // We're parsing a format specifier: %[flags][width][.precision][length]
    ADVANCE_IN_FORMAT_STRING(format);

    printf_flags_t flags = parse_flags(&format);

    // evaluate width field
    printf_size_t width = 0U;
    if (STDIO_ISDIGIT(*format)) {
      width = (printf_size_t)stdio_atou(&format);
    } else if (*format == '*') {
      const int w = va_arg(args, int);
      if (w < 0) {
        flags |= FLAGS_LEFT; // reverse padding
        width = (printf_size_t)-w;
      } else {
        width = (printf_size_t)w;
      }
      ADVANCE_IN_FORMAT_STRING(format);
    }

    // evaluate precision field
    printf_size_t precision = 0U;
    if (*format == '.') {
      flags |= FLAGS_PRECISION;
      ADVANCE_IN_FORMAT_STRING(format);
      if (STDIO_ISDIGIT(*format)) {
        precision = (printf_size_t)stdio_atou(&format);
      } else if (*format == '*') {
        const int precision_ = va_arg(args, int);
        precision = precision_ > 0 ? (printf_size_t)precision_ : 0U;
        ADVANCE_IN_FORMAT_STRING(format);
      }
    }

    // evaluate length field
    switch (*format) {
      case 'l':
        flags |= FLAGS_LONG;
        ADVANCE_IN_FORMAT_STRING(format);
        if (*format == 'l') {
          flags |= FLAGS_LONG_LONG;
          ADVANCE_IN_FORMAT_STRING(format);
        }
        break;
      case 'h':
        flags |= FLAGS_SHORT;
        ADVANCE_IN_FORMAT_STRING(format);
        if (*format == 'h') {
          flags |= FLAGS_CHAR;
          ADVANCE_IN_FORMAT_STRING(format);
        }
        break;
      case 't':
        flags |=
            (sizeof(ptrdiff_t) == sizeof(long) ? FLAGS_LONG : FLAGS_LONG_LONG);
        ADVANCE_IN_FORMAT_STRING(format);
        break;
      case 'j':
        flags |=
            (sizeof(intmax_t) == sizeof(long) ? FLAGS_LONG : FLAGS_LONG_LONG);
        ADVANCE_IN_FORMAT_STRING(format);
        break;
      case 'z':
        flags |= (sizeof(size_t) == sizeof(long) ? FLAGS_LONG : FLAGS_LONG_LONG);
        ADVANCE_IN_FORMAT_STRING(format);
        break;
      default:
        break;
    }

    // evaluate specifier
    switch (*format) {
      case 'd':
      case 'i':
      case 'u':
      case 'x':
      case 'X':
      case 'o':
      case 'b': {
        if (*format == 'd' || *format == 'i') {
          flags |= FLAGS_SIGNED;
        }

        numeric_base_t base;
        if (*format == 'x' || *format == 'X') {
          base = BASE_HEX;
        } else if (*format == 'o') {
          base = BASE_OCTAL;
        } else if (*format == 'b') {
          base = BASE_BINARY;
        } else {
          base = BASE_DECIMAL;
          flags &= ~FLAGS_HASH; // decimal integers have no alternative
              // presentation
        }

        if (*format == 'X') {
          flags |= FLAGS_UPPERCASE;
        }

        format++;
        // ignore '0' flag when precision is given
        if (flags & FLAGS_PRECISION) {
          flags &= ~FLAGS_ZEROPAD;
        }

        if (flags & FLAGS_SIGNED) {
          // A signed specifier: d, i or possibly I + bit size if enabled

          if (flags & FLAGS_LONG_LONG) {
#if RCSW_STDIO_PRINTF_WITH_LL
            const long long value = va_arg(args, long long);
            print_integer(output,
                          ABS_FOR_PRINTING(value),
                          value < 0,
                          base,
                          precision,
                          width,
                          flags);
#endif
          } else if (flags & FLAGS_LONG) {
            const long value = va_arg(args, long);
            print_integer(output,
                          ABS_FOR_PRINTING(value),
                          value < 0,
                          base,
                          precision,
                          width,
                          flags);
          } else {
            // We never try to interpret the argument as something
            // potentially-smaller than int, due to integer promotion rules: Even
            // if the user passed a short int, short unsigned etc. - these will
            // come in after promotion, as int's (or unsigned for the case of
            // short unsigned when it has the same size as int)
            const int value =
                (flags & FLAGS_CHAR)    ? (signed char)va_arg(args, int)
                : (flags & FLAGS_SHORT) ? (short int)va_arg(args, int)
                                        : va_arg(args, int);
            print_integer(output,
                          ABS_FOR_PRINTING(value),
                          value < 0,
                          base,
                          precision,
                          width,
                          flags);
          }
        } else {
          // An unsigned specifier: u, x, X, o, b

          flags &= ~(FLAGS_PLUS | FLAGS_SPACE);

          if (flags & FLAGS_LONG_LONG) {
#if RCSW_STDIO_PRINTF_WITH_LL
            print_integer(output,
                          (printf_unsigned_value_t)va_arg(args,
                                                          unsigned long long),
                          false,
                          base,
                          precision,
                          width,
                          flags);
#endif
          } else if (flags & FLAGS_LONG) {
            print_integer(output,
                          (printf_unsigned_value_t)va_arg(args, unsigned long),
                          false,
                          base,
                          precision,
                          width,
                          flags);
          } else {
            const unsigned int value =
                (flags & FLAGS_CHAR) ? (unsigned char)va_arg(args, unsigned int)
                : (flags & FLAGS_SHORT)
                    ? (unsigned short int)va_arg(args, unsigned int)
                    : va_arg(args, unsigned int);
            print_integer(output,
                          (printf_unsigned_value_t)value,
                          false,
                          base,
                          precision,
                          width,
                          flags);
          }
        }
        break;
      }
#if RCSW_STDIO_PRINTF_WITH_DEC
      case 'f':
      case 'F':
        if (*format == 'F')
          flags |= FLAGS_UPPERCASE;
        print_floating_point(output,
                             va_arg(args, double),
                             precision,
                             width,
                             flags,
                             PRINTF_PREFER_DECIMAL);
        format++;
        break;
#endif
#if RCSW_STDIO_PRINTF_WITH_EXP
      case 'e':
      case 'E':
      case 'g':
      case 'G':
        if ((*format == 'g') || (*format == 'G'))
          flags |= FLAGS_ADAPT_EXP;
        if ((*format == 'E') || (*format == 'G'))
          flags |= FLAGS_UPPERCASE;
        print_floating_point(output,
                             va_arg(args, double),
                             precision,
                             width,
                             flags,
                             PRINTF_PREFER_EXPONENTIAL);
        format++;
        break;
#endif
      case 'c': {
        printf_size_t l = 1U;
        // pre padding
        if (!(flags & FLAGS_LEFT)) {
          while (l++ < width) {
            gadget_putchar(output, ' ');
          }
        }
        // char output
        gadget_putchar(output, (char)va_arg(args, int));
        // post padding
        if (flags & FLAGS_LEFT) {
          while (l++ < width) {
            gadget_putchar(output, ' ');
          }
        }
        format++;
        break;
      }

      case 's': {
        const char* p = va_arg(args, char*);
        if (p == NULL) {
          out_reversed(output, ")llun(", 6, width, flags);
        } else {
          printf_size_t l =
              stdio_strnlen(p, precision ? precision : PRINTF_MAX_BUF_SIZE);
          // pre padding
          if (flags & FLAGS_PRECISION) {
            l = (l < precision ? l : precision);
          }
          if (!(flags & FLAGS_LEFT)) {
            while (l++ < width) {
              gadget_putchar(output, ' ');
            }
          }
          // string output
          while ((*p != 0) && (!(flags & FLAGS_PRECISION) || precision)) {
            gadget_putchar(output, *(p++));
            --precision;
          }
          // post padding
          if (flags & FLAGS_LEFT) {
            while (l++ < width) {
              gadget_putchar(output, ' ');
            }
          }
        }
        format++;
        break;
      }

      case 'p': {
        width = sizeof(void*) * 2U + 2; // 2 hex chars per byte + the "0x" prefix
        flags |= FLAGS_ZEROPAD | FLAGS_POINTER;
        uintptr_t value = (uintptr_t)va_arg(args, void*);
        (value == (uintptr_t)NULL) ? out_reversed(output, ")lin(", 5, width, flags)
                                   : print_integer(output,
                                                   (printf_unsigned_value_t)value,
                                                   false,
                                                   BASE_HEX,
                                                   precision,
                                                   width,
                                                   flags);
        format++;
        break;
      }

      case '%':
        gadget_putchar(output, '%');
        format++;
        break;

        // Many people prefer to disable support for %n, as it lets the caller
        // engineer a write to an arbitrary location, of a value the caller
        // effectively controls - which could be a security concern in some cases.
#if RCSW_STDIO_PRINTF_WITH_WRITEBACK
      case 'n': {
        if (flags & FLAGS_CHAR)
          *(va_arg(args, char*)) = (char)output->pos;
        else if (flags & FLAGS_SHORT)
          *(va_arg(args, short*)) = (short)output->pos;
        else if (flags & FLAGS_LONG)
          *(va_arg(args, long*)) = (long)output->pos;
#if RCSW_STDIO_PRINTF_WITH_LL
        else if (flags & FLAGS_LONG_LONG)
          *(va_arg(args, long long*)) = (long long int)output->pos;
#endif
        else
          *(va_arg(args, int*)) = (int)output->pos;
        format++;
        break;
      }
#endif

      default:
        gadget_putchar(output, *format);
        format++;
        break;
    }
  }
}

// internal vsnprintf - used for implementing _all library functions
static int vsnprintf_impl(struct printf_output_gadget* output,
                          const char* format,
                          va_list args) {
  // Note: The library only calls vsnprintf_impl() with output->pos being
  // 0. However, it is possible to call this function with a non-zero pos value
  // for some "remedial printing".
  format_string_loop(output, format, args);

  // termination
  gadget_append_termination(output);

  // return written chars without terminating \0
  return (int)output->pos;
}

/*******************************************************************************
 * API Functions
 ******************************************************************************/
int stdio_vprintf(const char* format, va_list arg) {
  struct printf_output_gadget gadget = gadget_putchar_extern();
  return vsnprintf_impl(&gadget, format, arg);
}

int stdio_vsnprintf(char* s, size_t n, const char* format, va_list arg) {
  struct printf_output_gadget gadget = gadget_init_with_buf(s, n);
  return vsnprintf_impl(&gadget, format, arg);
}

int stdio_vsprintf(char* s, const char* format, va_list arg) {
  return stdio_vsnprintf(s, PRINTF_MAX_BUF_SIZE, format, arg);
}

int stdio_vusfprintf(void (*out)(char c, void* extra_arg),
                     void* extra_arg,
                     const char* format,
                     va_list arg) {
  struct printf_output_gadget gadget = gadget_init_with_cb(out, extra_arg);
  return vsnprintf_impl(&gadget, format, arg);
}

int stdio_printf(const char* format, ...) {
  va_list args;
  va_start(args, format);
  const int ret = stdio_vprintf(format, args);
  va_end(args);
  return ret;
}

int stdio_sprintf(char* s, const char* format, ...) {
  va_list args;
  va_start(args, format);
  const int ret = stdio_vsprintf(s, format, args);
  va_end(args);
  return ret;
}

int stdio_snprintf(char* s, size_t n, const char* format, ...) {
  va_list args;
  va_start(args, format);
  const int ret = stdio_vsnprintf(s, n, format, args);
  va_end(args);
  return ret;
}

int stdio_usfprintf(void (*out)(char c, void* extra_arg),
                    void* extra_arg,
                    const char* format,
                    ...) {
  va_list args;
  va_start(args, format);
  const int ret = stdio_vusfprintf(out, extra_arg, format, args);
  va_end(args);
  return ret;
}

END_C_DECLS
