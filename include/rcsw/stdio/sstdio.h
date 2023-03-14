/**
 * \file sstdio.h
 * \ingroup sstdio
 * \brief Implementation of stdio libary.
 *
 * Contains mostly routines involved in supporting printf(). Should mainly be
 * used in bare-metal environments (i.e. no OS/stdlib, e.g.  bootstraps).
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef INCLUDE_RCSW_STDIO_SSTDIO_H_
#define INCLUDE_RCSW_STDIO_SSTDIO_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
/**
 * The standard stdarg.h provided here is only here as a means of
 * testing/validation. Realistically, when using this library you will probably
 * need to create your own/include the platform-specific version for your platform.
 */
#include <stdarg.h>
#include "rcsw/common/common.h"

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Print formatted strings to stdout.
 *
 * This routine supports standard format string/variable arg display of chars to
 * the terminal/UART. It supports almost the same set of format specifiers that
 * the regular printf() does.
 *
 *
 * If there are more than 1/2 * strlen(fmt) newlines in the string, stack
 * smashing will occur, because all newlines in the format string are replaced
 * with \\r\\n so that text displays correctly on raw serial terminals.
 *
 * \return The number of bytes written to stdout.
 */
int sstdio_printf(const char *fmt, ...) RCSW_ATTR(format(printf, 1, 2));

/**
 * \brief Print formatted strings to stdout.
 *
 * This routine supports standard format string/variable arg display of chars to
 * a terminal, but requires the use of va_args, instead of a variable number of
 * arguments ala printf(). It can be invoked directly, but is most commonly
 * invoked by \ref sstdio_printf().
 *
 * The following format modifiers are supported (All format modifiers must
 * appear before any format specifiers in the format block). As a general rule
 * mixing format modifiers should be done with care, because there are a lot of
 * combinations that I haven't tested...
 *
 * #  :     Add a 0x prefix. Only affects format blocks containing a x or X.
 *          Implies a fill character of 0; do not specify a fill char with #
 *          (i.e., #4x = add 0x prefix and zero-fill up to 4 printing digits,
 *          #04x is undefined.)
 *
 * pq :     Fill characters. Print q characters; if p is not present, numbers which
 *          ultimately render into < q digits will be space-filled. If p is 0,
 *          then numbers will be 0 filled. If the argument ultimately renders
 *          into > q digits, then the request to print only p of them is
 *          ignored. If fill characters are desired, then the pq format
 *          modifier must be specified as the last format modifier before the
 *          main format specifier (d,x,X are supported). If the argument is
 *          negative the sign character is included in the length of the
 *          rendered argument when determining the number of fill
 *          characters. The maximum number of fill chars is limited to 9.
 *
 * + :      Always include the sign specifier. Valid to be used with d,i format
 *          format specifiers. (Don't need to support x,X--hex numbers are
 *          ALWAYS display as positive numbers). If used in conjunction with
 *          character filling, the sign is include the sign character is
 *          included in the length of the rendered argument when determining
 *          the number of fill characters.
 *
 * -        Left-justify the result. Only really helpful with a width
 *          specifier, to pad on the right instead of the left. Only use with d.
 *
 * .pq :    Floating point precision. Display p characters after the decimal point
 *          if not enough characters exist, the result will be 0 padded. If the
 *          argument requires > p digits, then it is rounded appropriately to be
 *          displayed in p digits. Maximum of 8 digits. Valid to be used with f,
 *          e,E format specifiers.
 *
 * The following format specifiers are supported:
 * d       : Decimal signed integer (positive or negative)
 * x       : Hexadecimal, with a,b,c,d,e,f (positive only; neg numbers treated
 *                                          as size_t, i.e. -0x100 treated as
 *                                          0xffffff00)
 * X       : Hexadecimal, with A,B,C,D,E,F (positive only; neg numbers treated
 *                                          as size_t, i.e. -0x100 treated as
 *                                          0xffffff00)
 * c       : Character
 *
 * s       : String. Must be null-terminated.
 *
 * p       : Pointer. Note that the p and x format specifiers are NOT equivalent
 *           in general when rendering a pointer argument--they can be equivalent
 *           in some implementations. x casts the argument to an size_t integer,
 *           while p interpets it as is. On SPARC, p and x happen to behave the
 *           same; i.e., size_t int is 32 bits, and so is a pointer. On 64 bit
 *           x86, they are not equivalent. As a general rule use p for pointers
 *           and x/X for other size_t ints. Using this format specifier will
 *           automatically prepend 0x to the rendered argument.
 *
 * f       : Floating point. The argument is treated as a floating point number.
 *           If the argument is not a floating point number, then this doesn't
 *           work. If the number is really small or really large, then
 *           scientific notation will be used. If scientific notation is used a
 *           lowercase 'e' will be used for the exponent, and at least 2 digits
 *           will be used for the power. Unless the floating point decimal
 *           precision specifier is used, 6 digits will be displayed.
 *
 * e : Floating point, but force scientific notation, regardless of argument's
 * size. Uses 'e' as the exponent character.
 *
 * E : Same as 'e', but with an 'E' for an exponent.
 *
 * The following format specifiers are planned, but not yet implemented:
 * g       : use the shorter of f and e
 * G       : use the shorter of f and E
 * .qi     : Precision: minimum number of digits printed for integer. Zero fill.
 *
 * \param fmt The format string.
 * \param argp The list of arguments passed to printf().
 *
 * NOTES:
 * If the number you request to print is more digits than you requested to print
 * (i.e. you said %3d for a 5 digit integer) then your request will be ignored
 * and the full number of digits printed.
 *
 * \return The number of bytes written to stdout
 */
int sstdio_vprintf(const char *fmt, va_list argp);

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
size_t sstdio_puts(const char *const s);

/**
 * \brief Write a character to stdout.
 *
 * This routine is the implementation-specific "write a char to stdout"
 * function.  On x86, it is a wrapper around the putchar() function.
 *
 * \param c The char to write.
 */
static inline void sstdio_putchar(char c) { putchar(c); }

/**
 * \brief Get a character from stdin.
 *
 * This routine is the implementation-specific "get a char from stdin" function.
 * On x86, it is a wrapper() around the getchar() function.
 *
 * \return The character received.
 */
static inline int sstdio_getchar(void) { return getchar(); }

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
int sstdio_atoi(const char *s, int base) RCSW_PURE;

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
char *sstdio_itoad(int n, char *s);

/**
 * \brief Convert an integer into a hexadecimal string.
 *
 * This routine converts an integer into a hexadecimal string representing the
 * value of the original integer. If i is negative, it will be treated as
 * size_t.
 *
 * \param i The number to convert.
 * \param s The string to fill.
 *
 * \return The converted string.
 */
char *sstdio_itoax(int i, char *s);

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
char *sstdio_dtoa(double n, bool_t force_exp, char *s);

END_C_DECLS

#endif /*  INCLUDE_RCSW_STDIO_SSTDIO_H_  */
