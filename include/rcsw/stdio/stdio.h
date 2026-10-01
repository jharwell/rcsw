/**
 * \file
 *
 * \copyright 2017 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * \ingroup stdio
 *
 * \brief A small stdio library.
 *
 * Mostly routines that support printf(). Mainly for bare-metal environments
 * with no OS or stdlib, e.g. bootstraps.
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stddef.h>

#include "rcsw/al/types.h"
#include "rcsw/core/compilers.h"

/*******************************************************************************
 * Configuration
 ******************************************************************************/
BEGIN_C_DECLS

#ifndef RCSW_CONFIG_STDIO_PUTCHAR
#define RCSW_CONFIG_STDIO_PUTCHAR putchar
#endif
/**
 * \def RCSW_CONFIG_STDIO_PUTCHAR
 *
 * \brief The name of the putchar()-like function that writes one character to
 * stdout. Defaults to libc's \c putchar.
 *
 * In a shared build the function must be resolvable when RCSW itself is
 * linked; an application-provided function only works in a static build. See
 * \rcswdoc{concepts/components}.
 *
 * \note The declaration below is deliberately not weak: a weak declaration
 * with no definition resolves to address 0 and crashes when called.
 */
/** \cond INTERNAL */
RCSW_WARNING_DISABLE_PUSH()
RCSW_WARNING_DISABLE_REDUNDANT_DECLS()
RCSW_API int RCSW_CONFIG_STDIO_PUTCHAR(int c);
RCSW_WARNING_DISABLE_POP()
/** \endcond */

#ifndef RCSW_CONFIG_STDIO_GETCHAR
#define RCSW_CONFIG_STDIO_GETCHAR getchar
#endif

/**
 * \def RCSW_CONFIG_STDIO_GETCHAR
 *
 * \brief The name of the getchar()-like function that reads one character
 * from stdin. Defaults to libc's \c getchar.
 *
 * The same linking rules as \ref RCSW_CONFIG_STDIO_PUTCHAR apply.
 */
/** \cond INTERNAL */
RCSW_WARNING_DISABLE_PUSH()
RCSW_WARNING_DISABLE_REDUNDANT_DECLS()
RCSW_API int RCSW_CONFIG_STDIO_GETCHAR(void);
RCSW_WARNING_DISABLE_POP()
/** \endcond */

/*******************************************************************************
 * Public API
 ******************************************************************************/
/**
 * \brief Write a string to stdout.
 *
 * Unlike libc's \c puts(), no newline is appended.
 *
 * \param s The string to write.
 *
 * \return The number of bytes written.
 */
RCSW_API size_t stdio_puts(const char* s);

/**
 * \brief Write a character to stdout.
 *
 * Calls \ref RCSW_CONFIG_STDIO_PUTCHAR.
 *
 * \param c The char to write.
 *
 * \return What \ref RCSW_CONFIG_STDIO_PUTCHAR returns: \p c, or \c EOF on
 * error for libc's \c putchar.
 */
RCSW_API int stdio_putchar(int c);

/**
 * \brief Get a character from stdin.
 *
 * Calls \ref RCSW_CONFIG_STDIO_GETCHAR.
 *
 * \return The character received.
 */
RCSW_API int stdio_getchar(void);

/**
 * \brief Convert a string to an integer.
 *
 * Convert a string representing an integer in the specified base. Any leading
 * whitespace is stripped. If the string is a hex number, it must have a 0x
 * prefix. If the string represents a hex number but does not have a 0x prefix,
 * the result is undefined.
 *
 * \param s The string to convert
 * \param base The base the string is in (10, 16, etc.)
 *
 * \return The converted result. Values outside the range of int wrap.
 */
RCSW_API int stdio_atoi(const char* s, int base) RCSW_PURE;

/**
 * \brief Buffer size sufficient for any \ref stdio_itoad() result, including
 * the terminating NUL (sign + 10 digits + NUL).
 */
#define RCSW_STDIO_ITOAD_BUFSIZE 12

/**
 * \brief Buffer size sufficient for any \ref stdio_itoax() result, including
 * the "0x" prefix and the terminating NUL.
 */
#define RCSW_STDIO_ITOAX_BUFSIZE 11

/**
 * \brief Convert a 32-bit integer into a decimal string.
 *
 * Negative numbers are prefixed with '-', positive numbers with '+', and 0 has
 * no sign.
 *
 * \param n The number to convert.
 * \param s The buffer to fill.
 * \param len Size of \p s in bytes. \ref RCSW_STDIO_ITOAD_BUFSIZE is always
 *            sufficient.
 *
 * \return \p s, or NULL if \p s is NULL or \p len is too small for the
 * result (errno = ENOSPC); \p s is not modified in that case.
 */
RCSW_API char* stdio_itoad(int32_t n, char* s, size_t len);

/**
 * \brief Convert a 32-bit unsigned integer into a lowercase hexadecimal
 * string, without leading zeros.
 *
 * \param i The number to convert.
 * \param s The buffer to fill.
 * \param len Size of \p s in bytes. \ref RCSW_STDIO_ITOAX_BUFSIZE is always
 *            sufficient.
 * \param add_0x Should "0x" be added to the front of the string?
 *
 * \return \p s, or NULL if \p s is NULL or \p len is too small for the
 * result (errno = ENOSPC); \p s is not modified in that case.
 */
RCSW_API char* stdio_itoax(uint32_t i, char* s, size_t len, bool_t add_0x);

END_C_DECLS
