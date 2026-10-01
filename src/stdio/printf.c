/**
 * \file
 *
 * \copyright 2026 John Harwell
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/stdio/printf.h"

RCSW_WARNING_DISABLE_PUSH()
RCSW_WARNING_DISABLE_DOCUMENTATION()
#include <printf/printf.h>
RCSW_WARNING_DISABLE_POP()

/*******************************************************************************
 * API Functions
 ******************************************************************************/
int stdio_printf(const char* format, ...) {
  va_list args;
  va_start(args, format);
  int ret = vprintf_(format, args);
  va_end(args);
  return ret;
}

int stdio_vprintf(const char* format, va_list arg) {
  return vprintf_(format, arg);
}
int stdio_sprintf(char* s, const char* format, ...) {
  va_list args;
  va_start(args, format);
  int ret = vsprintf_(s, format, args);
  va_end(args);
  return ret;
}

int stdio_vsprintf(char* s, const char* format, va_list arg) {
  return vsprintf_(s, format, arg);
}

int stdio_snprintf(char* s, size_t n, const char* format, ...) {
  va_list args;
  va_start(args, format);
  int ret = vsnprintf_(s, n, format, args);
  va_end(args);
  return ret;
}

int stdio_vsnprintf(char* s, size_t count, const char* format, va_list arg) {
  return vsnprintf_(s, count, format, arg);
}

int stdio_usfprintf(void (*out)(char c, void* extra_arg),
                    void*       extra_arg,
                    const char* format,
                    ...) {
  va_list args;
  va_start(args, format);
  int ret = vfctprintf(out, extra_arg, format, args);
  va_end(args);
  return ret;
}

int stdio_vusfprintf(void (*out)(char c, void* extra_arg),
                     void*       extra_arg,
                     const char* format,
                     va_list     arg) {
  return vfctprintf(out, extra_arg, format, arg);
}
