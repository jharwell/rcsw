/**
 * \file string.h
 * \ingroup sstdio
 * \brief Implementation of stdio string routines.
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
 * Macros
 ******************************************************************************/
/* these macros operate on single characters */
#define RCSW_STDIO_ISPRINTABLE(c) (((c) >= ' ' && (c) <= '~') ? 1 : 0)
#define RCSW_STDIO_ISSPACE(c) (((c) == ' ') ? 1 : 0)
#define RCSW_STDIO_ISLOWER(c) (((c) >= 'a' && (c) <= 'z') ? 1 : 0)
#define RCSW_STDIO_ISUPPER(c) (((c) >= 'A' && (c) <= 'Z') ? 1 : 0)
#define RCSW_STDIO_ISDIGIT(c) (((c) >= '0' && (c) <= '9') ? 1 : 0)
#define RCSW_STDIO_ISHEX(c) (RCSW_STDIO_ISDIGIT(c) ||           \
                             ((c) >= 'a' && (c) <= 'f') ||              \
                             ((c) >= 'A' && (c) <= 'F') ? 1 : 0)
#define RCSW_STDIO_ISALPHA(c)                                           \
  ((((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z')) ? 1 : 0)

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Copy memory.
 *
 * This routine copies n bytes from the the memory pointed to by src to the
 * memory pointed to by dest. The memory areas must NOT overlap.
 *
 * \param dest Destination of copy.
 * \param src Source of copy.
 * \param n # of bytes to copy.
 *
 * \return Memory pointed to by dest.
 */
RCSW_API void* stdio_memcpy(void* __restrict__ dest,
                            const void* __restrict__ src,
                            size_t n);

/**
 * \brief Set memory.
 *
 * Fills the first \p n bytes of the memory area pointed to by \p dest with the
 * constant byte \p c.
 *
 * \param dest Destination of copy.
 * \param c The byte to use.
 * \param n # of bytes to copy.
 *
 * \return Memory pointed to by dest.
 */
RCSW_API void* stdio_memset(void* __restrict__ dest,
                            int c,
                            size_t n);

/**
 * \brief Convert a char to upper case.
 *
 * \param c The character to convert.
 *
 * \return The converted char.
 */
RCSW_API int stdio_toupper(int c) RCSW_CONST;

/**
 * \brief Convert a char to lower case.
 *
 * \param c The character to convert.
 *
 * \return The converted char.
 */
RCSW_API int stdio_tolower(int c) RCSW_CONST;

/**
 * \brief Get the length of a string.
 *
 * \param s The string
 *
 * \return The length of the string.
 */
RCSW_API size_t stdio_strlen(const char * s) RCSW_PURE;

/**
 * \brief Get the length of a string, limited by \p maxsize
 *
 * \param s The string
 *
 * \param maxsize The maximum string length to consider
 *
 * \return The length of the string.
 */

RCSW_API size_t stdio_strnlen(const char* const s, size_t maxsize) RCSW_PURE;

/**
 * \brief Reverse a string of known length.
 *
 * Reverses a string of known length in O(N) time, because there is no need to
 * call strlen() to find the length of the string. Modified the original string
 * is by this function.
 *
 * \param s The string to reverse.
 * \param len Length of the string to reverse.
 */
RCSW_API void stdio_strrev(char *s, size_t len);

/**
 * \brief Search a string for another string.
 *
 * This routine searches the haystack for the first occurence of the needle.
 * All bytes of needle must be found contiguously, including the null byte; that
 * is, the needle string MUST be null terminated. If it is not null terminated
 * then this routine will not be able to figure out where the needle string
 * ends. If more than one occurence of needle is found in haystack, the pointer
 * to the first one encountered will be returned.
 *
 * \param haystack The string to search IN.
 * \param needle The string to search for.
 *
 * \return: The first occurence of the needle, or NULL if the substring was not
 *          found.
 */
RCSW_API const char *stdio_strstr(const char * haystack,
                                  const char * needle) RCSW_PURE;

/**
 * \brief Search a string for a character.
 *
 * This routine searches the haystack for the first occurence of the needle.
 * The character must be a byte; that is, multi-byte characters are not
 * supported.  The haystack must be null terminated. If more than one occurence
 * of needle is found in the haystack, a pointer to the first one will be
 * returned.
 *
 * \param haystack The string to search IN.
 * \param needle The char to search for.
 *
 * \return Pointer to the first occurence of the needle, or NULL if the
 *         char was not found.
 */
RCSW_API const char *stdio_strchr(const char * haystack, char needle) RCSW_PURE;

/**
 * \brief Copy one string over another.
 *
 * This routine copies one string over top of another until the null byte is
 * found or until n bytes have been written, whichever comes first. As per the
 * standard library strncpy(), if strlen(haystack) < n, then the remaining bytes
 * in dest are filled with zeroes.
 *
 * \param dest Copy destination.
 * \param src Copy source.
 * \param n Max # of bytes to copy.
 *
 * \return Pointer to dest.
 */
RCSW_API char *stdio_strncpy(char * __restrict__ dest,
                             const char * __restrict__ src,
                             size_t n);

/**
 * \brief Copy one string over another.
 *
 * This routine copies one string over top of another until the NULL byte.
 *
 * The src/dest strings must NOT overlap. The null byte of src is also copied
 * into dest. If either \p src or \p dest is NULL, nothing is done.
 *
 * \param dest Copy destination.
 *
 * \param src Copy source.
 *
 * \return Pointer to dest, unless \p dest is NULL, then NULL.
 */
RCSW_API char *stdio_strcpy(char * __restrict__ dest,
                            const char * __restrict__ src);

/**
 * \brief Compare two strings for equality.
 *
 * This routine compares two strings alphabetically, char by char, to a
 * maximum of len chars.
 *
 * \param s1 String #1.
 * \param s2 String #2.
 * \param len # of bytes to compare.
 *
 * \return <,=,> 0 depending if s1 is found to be <,=,> s2
 */
RCSW_API int stdio_strncmp(const char * s1,
                           const char * s2,
                           size_t len) RCSW_PURE;

/**
 * \brief Compare two strings for equality.
 *
 * This routine compares two strings alphabetically, char by char, until
 * the NULL byte is encountered.
 *
 * \param s1 String #1.
 * \param s2 String #2.
 *
 * \return <,=,> 0 depending if s1 is found to be <,=,> s2
 */
RCSW_API int stdio_strcmp(const char * s1, const char * s2) RCSW_PURE;

/**
 * \brief Replace a occurrences of one substring within another string with
 * a new substring.
 *
 * If new is not large enough to hold the new string formed by replacing all
 * occurrences of pattern with replacement, undefined behavior will occur.
 *
 * \param original The string to replacement stuff in.
 * \param pattern The substring to replace.
 * \param replacement The substring to replace occurrences of pattern with.
 * \param new_str The updated string.
 *
 * \return The updated string.
 */
RCSW_API char *stdio_strrep(const char * __restrict__ original,
                            const char * __restrict__ pattern,
                            const char * __restrict__ replacement,
                            char * __restrict__ new_str);
END_C_DECLS
