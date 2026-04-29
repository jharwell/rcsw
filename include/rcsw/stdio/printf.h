/**
 * \file
 *
 * \copyright 2017 John Harwell, All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * \brief Wrapper over the eyalroz/printf library, providing the RCSW
 * stdio_printf() family of functions.
 *
 * This header adapts the upstream eyalroz/printf API to the RCSW naming
 * convention and type system. Application code should include this header
 * rather than the upstream <printf.h> directly.
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdarg.h>

#include <eyalroz/printf.h>

#include "rcsw/core/compilers.h"

/*******************************************************************************
 * Public API
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief An implementation of the C standard's printf().
 *
 * \note Relies on \ref stdio_putchar() which must be provided by the
 * platform/BSP. This decouples printf() from OS/hardware details of how to
 * emit a character to stdout.
 *
 * \param format A string specifying the format of the output, with %-marked
 *               specifiers of how to interpret additional arguments.
 * \param ...    Additional arguments, one for each %-specifier in \p format.
 *
 * \return The number of characters written, not counting the terminating NUL.
 */
RCSW_API RCSW_ATTR_PRINTF(1, 2) static inline int stdio_printf(const char* format,
                                                               ...) {
  va_list args;
  va_start(args, format);
  int ret = vprintf_(format, args);
  va_end(args);
  return ret;
}

/**
 * \brief Same as \ref stdio_printf(), but accepts a va_list directly.
 *
 * \param format A format string.
 * \param arg    A va_list of arguments corresponding to \p format.
 *
 * \return The number of characters written, not counting the terminating NUL.
 */
RCSW_API static inline int stdio_vprintf(const char* format, va_list arg)
  RCSW_ATTR_PRINTF(1, 0);
static inline int stdio_vprintf(const char* format, va_list arg) {
  return vprintf_(format, arg);
}

/**
 * \brief An implementation of the C standard's sprintf().
 *
 * \warning Does not bounds-check \p s. Prefer \ref stdio_snprintf().
 *
 * \param s      Destination buffer. Must be large enough to hold the formatted
 *               output including the terminating NUL.
 * \param format A format string.
 * \param ...    Additional arguments, one for each %-specifier in \p format.
 *
 * \return The number of characters written into \p s, not counting the
 *         terminating NUL.
 */
RCSW_API static inline int stdio_sprintf(char* s, const char* format, ...)
  RCSW_ATTR_PRINTF(2, 3);
static inline int stdio_sprintf(char* s, const char* format, ...) {
  va_list args;
  va_start(args, format);
  int ret = vsprintf_(s, format, args);
  va_end(args);
  return ret;
}

/**
 * \brief Same as \ref stdio_sprintf(), but accepts a va_list directly.
 *
 * \param s      Destination buffer.
 * \param format A format string.
 * \param arg    A va_list of arguments corresponding to \p format.
 *
 * \return The number of characters written into \p s, not counting the
 *         terminating NUL.
 */
RCSW_API static inline int stdio_vsprintf(char*       s,
                                          const char* format,
                                          va_list     arg) RCSW_ATTR_PRINTF(2, 0);
static inline int stdio_vsprintf(char* s, const char* format, va_list arg) {
  return vsprintf_(s, format, arg);
}

/**
 * \brief An implementation of the C standard's snprintf().
 *
 * \param s      Destination buffer, or NULL to count characters only.
 * \param n      Maximum number of characters to write, including the
 *               terminating NUL.
 * \param format A format string.
 * \param ...    Additional arguments, one for each %-specifier in \p format.
 *
 * \return The number of characters that COULD have been written (excluding the
 *         terminating NUL). A value >= \p n indicates truncation.
 */
RCSW_API static inline int stdio_snprintf(char*       s,
                                          size_t      n,
                                          const char* format,
                                          ...) RCSW_ATTR_PRINTF(3, 4);
static inline int stdio_snprintf(char* s, size_t n, const char* format, ...) {
  va_list args;
  va_start(args, format);
  int ret = vsnprintf_(s, n, format, args);
  va_end(args);
  return ret;
}

/**
 * \brief Same as \ref stdio_snprintf(), but accepts a va_list directly.
 *
 * \param s      Destination buffer, or NULL to count characters only.
 * \param count  Maximum number of characters to write, including terminating
 *               NUL.
 * \param format A format string.
 * \param arg    A va_list of arguments corresponding to \p format.
 *
 * \return The number of characters that COULD have been written. A value >=
 *         \p count indicates truncation.
 */
RCSW_API static inline int stdio_vsnprintf(char*       s,
                                           size_t      count,
                                           const char* format,
                                           va_list arg) RCSW_ATTR_PRINTF(3, 0);
static inline int          stdio_vsnprintf(char*       s,
                                           size_t      count,
                                           const char* format,
                                           va_list     arg) {
  return vsnprintf_(s, count, format, arg);
}

/**
 * \brief printf() with a caller-supplied output function (USF = User Specified
 * Function).
 *
 * An alternative to \ref stdio_printf() in which the output function is
 * specified dynamically rather than using \ref stdio_putchar().
 *
 * \param out       Output function called once per character. Receives the
 *                  character and \p extra_arg.
 * \param extra_arg Opaque argument forwarded to \p out on every call.
 * \param format    A format string.
 * \param ...       Additional arguments, one for each %-specifier in \p
 *                  format.
 *
 * \return The number of characters for which \p out was invoked, not counting
 *         the terminating NUL.
 */
RCSW_API RCSW_ATTR_PRINTF(3, 4) static inline int stdio_usfprintf(
  void (*out)(char c, void* extra_arg),
  void*       extra_arg,
  const char* format,
  ...) {
  va_list args;
  va_start(args, format);
  int ret = vfctprintf(out, extra_arg, format, args);
  va_end(args);
  return ret;
}

/**
 * \brief Same as \ref stdio_usfprintf(), but accepts a va_list directly.
 *
 * \param out       Output function called once per character.
 * \param extra_arg Opaque argument forwarded to \p out on every call.
 * \param format    A format string.
 * \param arg       A va_list of arguments corresponding to \p format.
 *
 * \return The number of characters for which \p out was invoked, not counting
 *         the terminating NUL.
 */
RCSW_API RCSW_ATTR_PRINTF(3, 0) static inline int stdio_vusfprintf(
  void (*out)(char c, void* extra_arg),
  void*       extra_arg,
  const char* format,
  va_list     arg) {
  return vfctprintf(out, extra_arg, format, arg);
}

END_C_DECLS
