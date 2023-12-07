/**
 * \file stdio.h
 * \ingroup stdio
 * \brief Implementation of stdio libary.
 *
 * Contains mostly routines involved in supporting printf(). Should mainly be
 * used in bare-metal environments (i.e. no OS/stdlib, e.g.  bootstraps).
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/rcsw.h"

/*******************************************************************************
 * Configuration
 ******************************************************************************/
BEGIN_C_DECLS

#ifndef RCSW_CONFIG_STDIO_PUTCHAR
#define RCSW_CONFIG_STDIO_PUTCHAR putchar
#endif
/**
 * \brief RCSW_CONFIG_STDIO_PUTCHAR
 *
 * The name of the putchar()-like function to send a character to stdout in some
 * way.
 *
 * \note We don't put a weak attribute on the decl of the putchar() function
 * declaration, because that tells the linker "don't try to resolve this
 * symbol", which it happily obeys, resulting in a NULL pointer
 * dereference/segfault when the function is called if RCSW is built as a static
 * library. If it is built as a dynamic library, normal putchar() should be
 * available.
 */
RCSW_WARNING_DISABLE_PUSH()
RCSW_WARNING_DISABLE_REDUNDANT_DECLS()
RCSW_API int RCSW_CONFIG_STDIO_PUTCHAR(int c);
RCSW_WARNING_DISABLE_POP()

#ifndef RCSW_CONFIG_STDIO_GETCHAR
#define RCSW_CONFIG_STDIO_GETCHAR getchar
#endif

/**
 * \brief RCSW_CONFIG_STDIO_GETCHAR
 *
 * The name of the getchar()-like function to get a character from stdout in
 * some way.
 *
 * \note We don't put a weak attribute on the decl of the getchar() function
 * declaration, because that tells the linker "don't try to resolve this
 * symbol", which it happily obeys, resulting in a NULL pointer
 * dereference/segfault when the function is called if RCSW is built as a static
 * library. If it is built as a dynamic library, normal getchar() should be
 * available.
 */
RCSW_WARNING_DISABLE_PUSH()
RCSW_WARNING_DISABLE_REDUNDANT_DECLS()
RCSW_API int RCSW_CONFIG_STDIO_GETCHAR(void);
RCSW_WARNING_DISABLE_POP()

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/

/**
 * \brief Write a string to stdout.
 *
 * Write a string to stdout WITHOUT a newline (this is different than the GNUC
 * version).
 *
 * \param s The string to write.
 *
 * \return The number of bytes written.
 */
RCSW_API size_t stdio_puts(const char *s);

/**
 * \brief Write a character to stdout.
 *
 * The implementation-specific "write a char to stdout" function which uses \ref
 * RCSW_CONFIG_STDIO_PUTCHAR().
 *
 * \param c The char to write.
 */
RCSW_API int stdio_putchar(int c);

/**
 * \brief Get a character from stdin.
 *
 * The implementation-specific "get a char from stdin" function which uses \ref
 * RCSW_CONFIG_STDIO_GETCHAR().
 *
 * \return The character received.
 */
RCSW_API int stdio_getchar(void);

/**
 * \brief Convert a string to a integer.
 *
 * Convert a string representing an integer in the specified base. Any leading
 * whitespace is stripped.  If the string is a hex number, it must have a 0x
 * prefix. If the string represents a hex number but does not have a 0x prefix,
 * the result is undefined.
 *
 * \param s The string to convert
 * \param base The base the string is in (10, 16, etc.)
 *
 * \return The converted result.
 *
 * \todo This does not currently work if the string to be converted is INT_MIN.
 */
RCSW_API int stdio_atoi(const char *s, int base) RCSW_PURE;

/**
 * \brief Convert an integer into a decimal string.
 *
 * Convert an integer into a decimal string representing the value of the
 * original integer. Supports negative numbers. For uniformity, a '+' sign is
 * prepended to the string of all converted positive numbers. Only supports
 * 32-bit integers.
 *
 * \param n The number to convert
 * \param s The string to fill.
 *
 * \return The converted string.
 */
RCSW_API char *stdio_itoad(int32_t n, char *s);

/**
 * \brief Convert an integer into a hexadecimal string.
 *
 * Convert an integer into a hexadecimal string representing the value of the
 * original integer. If i is negative, it will be treated as size_t. Only
 * supports 32-bit integers.
 *
 * \param i The number to convert.
 * \param s The string to fill.
 * \param add_0x Should "0x" be added to the front of the string?
 *
 * \return The converted string.
 */
RCSW_API char *stdio_itoax(uint32_t i, char *s, bool_t add_0x);

END_C_DECLS
