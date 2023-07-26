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

#ifndef RCSW_STDIO_PUTCHAR
#define RCSW_STDIO_PUTCHAR_UNDEF
#else
/**
 * \brief RCSW_STDIO_PUTCHAR
 *
 * The name of the putchar()-like function to send a character to stdout in some
 * way.
 */
int RCSW_STDIO_PUTCHAR(int c);
#endif

#ifndef RCSW_STDIO_GETCHAR
#define RCSW_STDIO_GETCHAR_UNDEF
#else
/**
 * \brief RCSW_STDIO_GETCHAR
 *
 * The name of the getchar()-like function to get a character from stdout in
 * some way.
 */
int RCSW_STDIO_GETCHAR(void);
#endif

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/

/**
 * \brief Write a string to stdout.
 *
 * This routine writes a string to stdout WITHOUT a newline (this is different
 * than the GNUC version).
 *
 * \param s The string to write.
 *
 * \return The number of bytes written
 */
size_t stdio_puts(const char *const s);

/**
 * \brief Write a character to stdout.
 *
 * This routine is the implementation-specific "write a char to stdout"
 * function.  On x86, it is a wrapper around the putchar() function.
 *
 * \param c The char to write.
 */
#if !defined(RCSW_STDIO_PUTCHAR_UNDEF)
static inline int stdio_putchar(int c) {
  return RCSW_STDIO_PUTCHAR(c);
}
#else
static inline void stdio_putchar(int c) {}
#endif

/**
 * \brief Get a character from stdin.
 *
 * This routine is the implementation-specific "get a char from stdin" function.
 * On x86, it is a wrapper() around the getchar() function.
 *
 * \return The character received.
 */
#if !defined(RCSW_STDIO_GETCHAR_UNDEF)
static inline int stdio_getchar(void) {
  return RCSW_STDIO_GETCHAR();
}
#endif

/**
 * \brief Convert a string to a integer.
 *
 * This routine converts a string representing an integer in the specified
 * base. Any leading whitespace is stripped.  If the string is a hex number, it
 * must have a 0x prefix. If the string represents a hex number but does not
 * have a 0x prefix, the result is undefined.
 *
 * \param s The string to convert
 * \param base The base the string is in (10, 16, etc.)
 *
 * \return The converted result.
 *
 * \todo This does not currently work if the string to be converted is INT_MIN.
 */
int stdio_atoi(const char *s, int base) RCSW_PURE;

/**
 * \brief Convert an integer into a decimal string.
 *
 * This routine converts an integer into a decimal string representing the value
 * of the original integer. Negative numbers are supported. For uniformity, a
 * '+' sign is prepended to the string of all converted positive numbers. Only
 * supports 32-bit integers.
 *
 * \param n The number to convert
 * \param s The string to fill.
 *
 * \return The converted string.
 */
char *stdio_itoad(int32_t n, char *s);

/**
 * \brief Convert an integer into a hexadecimal string.
 *
 * This routine converts an integer into a hexadecimal string representing the
 * value of the original integer. If i is negative, it will be treated as
 * size_t. Only supports 32-bit integers.
 *
 * \param i The number to convert.
 * \param s The string to fill.
 * \param add_0x Should "0x" be added to the front of the string?
 *
 * \return The converted string.
 */
char *stdio_itoax(uint32_t i, char *s, bool_t add_0x);

END_C_DECLS
