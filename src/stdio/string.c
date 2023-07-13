/**
 * \file string.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#if !defined(__nos__)
#include <stdlib.h>
#endif

#include "rcsw/common/fpc.h"
#include "rcsw/stdio/string.h"

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

char* stdio_strrep(const char* const __restrict__ original,
                   const char* const __restrict__ pattern,
                   const char* const __restrict__ replacement,
                   char* const __restrict__ new_str) {
  size_t orilen = stdio_strlen(original);
  size_t replen = stdio_strlen(replacement);
  size_t patlen = stdio_strlen(pattern);
  const char* oriptr;
  const char* patloc;
  size_t patcnt = 0;

  /* find how many times the pattern occurs in the original string */
  for (oriptr = original; (patloc = stdio_strstr(oriptr, pattern));
       oriptr = patloc + patlen) {
    patcnt++;
  }

  /* allocate memory for the new string */
  size_t newlen = orilen + patcnt * (replen - patlen);
  new_str[newlen] = '\0';

  /* copy the original string, replacing all the instances of the pattern */
  char* retptr = new_str;
  for (oriptr = original; (patloc = stdio_strstr(oriptr, pattern));
       oriptr = patloc + patlen) {
    size_t skiplen = (size_t)(patloc - oriptr);

    /* copy the section until the occurence of the pattern */
    stdio_strncpy(retptr, oriptr, skiplen);
    retptr += skiplen;

    /* copy the replacement */
    stdio_strncpy(retptr, replacement, replen);
    retptr += replen;
  }

  /* copy the rest of the string */
  stdio_strcpy(retptr, oriptr);
  return new_str;
} /* stdio_strrep() */

void stdio_strrev(char* const s, size_t len) {
  int i = 0, j = len - 1; /* account for null byte */

  for (; i < j; i++, j--) {
    /* swap without temporary because I can */
    s[i] ^= s[j];
    s[j] ^= s[i];
    s[i] ^= s[j];
  }
} /* stdio_strrev() */

size_t stdio_strlen(const char* const s) {
  char const* p;
  for (p = s; *p; p++) {
  }

  return (size_t)(p - s);
} /* stdio_strlen() */

size_t stdio_strnlen(const char* const s, size_t maxsize) {
  char const* p;
  for (p = s; *p && maxsize--; p++) {
  }

  return (size_t)(p - s);
} /* stdio_strlen() */

const char* stdio_strchr(const char* haystack, char needle) {
  while (haystack != NULL && *haystack) {
    if (*haystack == needle) {
      return (const char*)haystack;
    }
    haystack++;
  }
  return NULL;
} /* stdio_strchr() */

const char* stdio_strstr(const char* const __restrict__ haystack,
                         const char* const __restrict__ needle) {
  const char* p1 = (const char*)haystack;
  if (!*needle) { /* null string */
    return p1;
  }

  while (*p1) { /* while there are chars left to check in haystack */
    const char* p1_curr = p1;
    const char* p2 = (const char*)needle;
    while (*p1 && *p2 && *p1 == *p2) { /* superimpose substring on current
                                        * position and check char by char */
      p1++;
      p2++;
    }
    if (!*p2) { /* All bytes in haystack match up until the null byte of needle,
                 * therefore we have a match. */
      return p1_curr;
    }
    p1 = p1_curr + 1; /* move starting position forward */
  }
  return NULL;
} /* stdio_strstr() */

char* stdio_strncpy(char* const __restrict__ dest,
                    const char* const __restrict__ src,
                    size_t n) {
  size_t i;
  /* copy up to null terminator, or n chars, whichever comes first */
  for (i = 0; i < n && src[i] != '\0'; i++) {
    dest[i] = src[i];
  }

  /* fill remaining space with 0's */
  for (; i < n; dest[i] = '\0', i++) {
    dest[i] = '\0';
  }
  return dest;
} /* stdio_strncpy() */

char* stdio_strcpy(char* __restrict__ dest, const char* const __restrict__ src) {
  RCSW_FPC_NV(dest, NULL != dest, NULL != src);

  size_t i;
  /* copy up to null terminator */
  for (i = 0; src[i] != '\0'; i++) {
    dest[i] = src[i];
  }

  dest[i] = '\0';
  return (char*)dest;
} /* stdio_strcpy() */

int stdio_strcmp(const char* const s1, const char* const s2) {
  const char* t1 = (const char*)s1;
  const char* t2 = (const char*)s2;
  while (*t1 == *t2) {
    if (*t1 == '\0') {
      return 0;
    }
    t1++;
    t2++;
  }
  return (*s1 - *s2);
} /* stdio_strcmp() */

int stdio_strncmp(const char* const s1, const char* const s2, size_t len) {
  size_t i = 0;
  const char* t1 = (const char*)s1;
  const char* t2 = (const char*)s2;

  /* special case: if len == 0, then by definition all strings are equivalent
   * up to the zeroth character */
  if (len == 0) {
    return 0;
  }

  while (*t1 == *t2) {
    if (i == len) {
      return 0;
    }
    i++;
    if (i == len) {
      break;
    }
    t1++;
    t2++;
  }
  return (*s1 - *s2);
} /* stdio_strncmp() */

int stdio_tolower(int c) {
  if (c >= 'A' && c <= 'Z') {
    c += ('a' - 'A');
  }
  return c;
} /* stdio_tolower() */

int stdio_toupper(int c) {
  if (c >= 'a' && c <= 'z') {
    c += ('A' - 'a');
  }
  return c;
} /* stdio_toupper() */

void* stdio_memcpy(void* const __restrict__ dest,
                   const void* const __restrict__ src,
                   size_t n) {
  char* d = dest;
  const char* s = src;
  for (size_t i = 0; i < n; i++) {
    d[i] = s[i];
  }
  return dest;
} /* stdio_memcpy() */

END_C_DECLS
