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
#warning "RCSW_STDIO_PUTCHAR not defined"
#define RCSW_STDIO_PUTCHAR_UNDEF
#else
void RCSW_STDIO_PUTCHAR(char);
#endif

#ifndef RCSW_STDIO_GETCHAR
#warning "RCSW_STDIO_GETCHAR not defined"
#define RCSW_STDIO_GETCHAR_UNDEF
#else
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
static inline void stdio_putchar(char c) {
  RCSW_STDIO_PUTCHAR(c);
}
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
 * '+' sign is prepended to the string of all converted positive numbers.
 *
 * \param n The number to convert
 * \param s The string to fill.
 *
 * \return The converted string.
 */
char *stdio_itoad(int n, char *s);

/**
 * \brief Convert an integer into a hexadecimal string.
 *
 * This routine converts an integer into a hexadecimal string representing the
 * value of the original integer. If i is negative, it will be treated as
 * size_t.
 *
 * \param i The number to convert.
 * \param s The string to fill.
 * \params add_0x Should "0x" be added to the front of the string?
 *
 * \return The converted string.
 */
char *stdio_itoax(int i, char *s, bool_t add_0x);

/**
 * \brief Convert a double into a string.
 *
 * This routine converts a double into a string representing the value
 * of the original double. Negative numbers are supported.
 *
 * If -10 < n < 10 and n is a whole number, and scientific notation is forced,
 * then the result will be something like 1 -> 1e+0 or -2 -> -2e+0. The lack of
 * a decimal point is due to the algorithm used, and I can't figure out how to
 * make this corner case work without screwing up the general case.
 *
 * \param n The number to convert.
 * \param force_exp If TRUE, scientific notation will always be used.
 * \param s The string to fill.
 *
 * RETURN:
 *     char* - The converted string.
 */
char *stdio_dtoa(double n, bool_t force_exp, char *s);


END_C_DECLS
