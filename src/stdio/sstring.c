/**
 * \file sstring.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * This file is part of RCSW.
 *
 * RCSW is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * RCSW is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * RCSW.  If not, see <http://www.gnu.org/licenses/
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#if !defined(__nos__)
#include <stdlib.h>
#endif
#include "rcsw/stdio/sstring.h"

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

char* sstring_strrep(const char* const __restrict__ original,
                     const char* const __restrict__ pattern,
                     const char* const __restrict__ replacement,
                     char* const __restrict__ new_str) {
  size_t orilen = sstring_strlen(original);
  size_t replen = sstring_strlen(replacement);
  size_t patlen = sstring_strlen(pattern);
  const char* oriptr;
  const char* patloc;
  size_t patcnt = 0;

  /* find how many times the pattern occurs in the original string */
  for (oriptr = original; (patloc = sstring_strstr(oriptr, pattern));
       oriptr = patloc + patlen) {
    patcnt++;
  }

  /* allocate memory for the new string */
  size_t newlen = orilen + patcnt * (replen - patlen);
  new_str[newlen] = '\0';

  /* copy the original string, replacing all the instances of the pattern */
  char* retptr = new_str;
  for (oriptr = original; (patloc = sstring_strstr(oriptr, pattern));
       oriptr = patloc + patlen) {
    size_t skiplen = (size_t)(patloc - oriptr);

    /* copy the section until the occurence of the pattern */
    sstring_strncpy(retptr, oriptr, skiplen);
    retptr += skiplen;

    /* copy the replacement */
    sstring_strncpy(retptr, replacement, replen);
    retptr += replen;
  }

  /* copy the rest of the string */
  sstring_strcpy(retptr, oriptr);
  return new_str;
} /* sstring_strrep() */

void sstring_strrev(char* const s, size_t len) {
  int i = 0, j = len - 1; /* account for null byte */

  for (; i < j; i++, j--) {
    /* swap without temporary because I can */
    s[i] ^= s[j];
    s[j] ^= s[i];
    s[i] ^= s[j];
  }
} /* sstring_strrev() */

size_t sstring_strlen(const char* const s) {
  char const* p = s;
  for (; *p != '\0'; p++) {
  }

  return (size_t)(p - s);
} /* sstring_strlen() */

const char* sstring_strchr(const char* haystack, char needle) {
  while (haystack != NULL && *haystack) {
    if (*haystack == needle) {
      return (const char*)haystack;
    }
    haystack++;
  }
  return NULL;
} /* sstring_strchr() */

const char* sstring_strstr(const char* const __restrict__ haystack,
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
} /* sstring_strstr() */

char* sstring_strncpy(char* const __restrict__ dest,
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
} /* sstring_strncpy() */

char* sstring_strcpy(char* __restrict__ dest,
                     const char* const __restrict__ src) {
  size_t i;
  /* copy up to null terminator */
  for (i = 0; src[i] != '\0'; i++) {
    dest[i] = src[i];
  }

  dest[i] = '\0';
  return (char*)dest;
} /* sstring_strcpy() */

int sstring_strcmp(const char* const s1, const char* const s2) {
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
} /* sstring_strcmp() */

int sstring_strncmp(const char* const s1, const char* const s2, size_t len) {
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
} /* sstring_strncmp() */

size_t sstring_num_digits(int a) {
  if (a == 0) {
    return 1;
  }

  if (a < 0) {
    a = -a;
  }

  int count = 0;
  while (a) {
    a /= 10;
    count++;
  }

  return count;
} /* sstring_num_digits */

int sstring_tolower(int c) {
  if (c >= 'A' && c <= 'Z') {
    c += ('a' - 'A');
  }
  return c;
} /* sstring_tolower() */

int sstring_toupper(int c) {
  if (c >= 'a' && c <= 'z') {
    c += ('A' - 'a');
  }
  return c;
} /* sstring_toupper() */

void* sstring_memcpy(void* const __restrict__ dest,
                     const void* const __restrict__ src,
                     size_t n) {
  char* d = dest;
  const char* s = src;
  for (size_t i = 0; i < n; i++) {
    d[i] = s[i];
  }
  return dest;
} /* sstring_memcpy() */

END_C_DECLS
