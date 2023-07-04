/**
 * \file printf.h
 *
 * \author (c) Eyal Rozenberg <eyalroz1\gmx.com>
 *             2021-2022, Haifa, Palestine/Israel
 * \author (c) Marco Paland (info\paland.com)
 *             2014-2019, PALANDesign Hannover, Germany
 *
 * \note Others have made smaller contributions to this file: see the
 * contributors page at https://github.com/eyalroz/printf/graphs/contributors
 * or ask one of the authors. The original code for exponential specifiers was
 * contributed by Martijn Jasperse <m.jasperse@gmail.com>.
 *
 * \brief Small stand-alone implementation of the printf family of functions
 * (`(v)printf`, `(v)s(n)printf` etc., geared towards use on embedded systems with
 * a very limited resources.
 *
 * SPDX-License Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdarg.h>
#include "rcsw/common/common.h"

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS
/**
 * An implementation of the C standard's printf()
 *
 * \note This function relies on an implementation of \ref RCSW_STDIO_PUTCHAR()
 * which you must provide; this decouples printf() from OS/hw details of exactly
 * _how_ to send a char to stdout.
 *
 * \param format A string specifying the format of the output, with %-marked
 * specifiers of how to interpret additional arguments.
 *
 * \param arg Additional arguments to the function, one for each %-specifier in
 * \p format string
 *
 * \return The number of characters written into \p s, not counting the
 * terminating null character
 */
int stdio_printf(const char* format, ...) RCSW_ATTR_PRINTF(1, 2);

/**
 * \brief Same as \ref stdio_printf(), but you pass a va_list directly.
 */
int stdio_vprintf(const char* format, va_list arg) RCSW_ATTR_PRINTF(1, 0);

/**
 * An implementation of the C standard's sprintf()
 *
 * \param s An array in which to store the formatted string. It must be large
 * enough to fit the formatted output!
 *
 * \param format A string specifying the format of the output, with %-marked
 * specifiers of how to interpret additional arguments.
 *
 * \param arg Additional arguments to the function, one for each specifier in \p
 * format
 *
 * \note To avoid buffer overflows, you should generally use \ref
 * stdio_snprintf() instead.
 *
 * \return The number of characters written into \p s, not counting the
 * terminating null character
 */
int stdio_sprintf(char* s, const char* format, ...) RCSW_ATTR_PRINTF(2, 3);

/**
 * \brief Same as \ref stdio_sprintf(), but you pass a va_list directly.
 */
int stdio_vsprintf(char* s,
                   const char* format,
                   va_list arg) RCSW_ATTR_PRINTF(2, 0);

/**
 * An implementation of the C standard's snprintf()
 *
 * \param s An array in which to store the formatted string. It must be large
 * enough to fit either the entire formatted output, or at least \p n
 * characters. Alternatively, it can be NULL, in which case nothing will be
 * printed, and only the number of characters which _could_ have been printed is
 * tallied and returned.
 *
 * \param n The maximum number of characters to write to the array, including a
 * terminating null character \param format A string specifying the format of
 * the output, with %-marked specifiers of how to interpret additional
 * arguments.
 *
 * \param arg Additional arguments to the function, one for each specifier in \p
 * format
 *
 * \return The number of characters that COULD have been written into \p s, not
 *         counting the terminating null character. A value equal or larger than
 *         \p n indicates truncation. Only when the returned value is
 *         non-negative and less than \p n, the null-terminated string has been
 *         fully and successfully printed.
 */
int  stdio_snprintf(char* s,
                    size_t count,
                    const char* format,
                    ...) RCSW_ATTR_PRINTF(3, 4);

/**
 * \brief Same as \ref stdio_snprintf(), but you pass a va_list directly.
 */
int stdio_vsnprintf(char* s,
                    size_t count,
                    const char* format,
                    va_list arg) RCSW_ATTR_PRINTF(3, 0);

/**
 * printf/vprintf with user-specified output function
 *
 * An alternative to \ref printf_, in which the output function is specified
 * dynamically (rather than \ref putchar_ being used)
 *
 * \param out An output function which takes one character and a type-erased
 * additional parameters
 *
 * \param extra_arg The type-erased argument to pass to the output function \p
 * out with each call
 *
 * \param format A string specifying the format of the output, with %-marked
 * specifiers of how to interpret additional arguments.
 *
 * \param arg Additional arguments to the function, one for each specifier in \p
 * format
 *
 * \return The number of characters for which the output f unction was invoked,
 * not counting the terminating null character
 */
int stdio_usfprintf(void (*out)(char c, void* extra_arg),
                    void* extra_arg,
                    const char* format,
                    ...) RCSW_ATTR_PRINTF(3, 4);

/**
 * \brief Same as \ref stdio_usfprintf(), but you pass a va_list directly.
 */
int stdio_vusfprintf(void (*out)(char c, void* extra_arg),
                     void* extra_arg,
                     const char* format,
                     va_list arg) RCSW_ATTR_PRINTF(3, 0);

END_C_DECLS
