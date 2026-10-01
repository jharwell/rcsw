/**
 * \file
 *
 * \copyright 2017 John Harwell
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/stdio/string.h"

#include "rcsw/core/fpc.h"

/*******************************************************************************
 * Public API
 ******************************************************************************/
BEGIN_C_DECLS

char* stdio_strrep(const char* const __restrict__ original,
                   const char* const __restrict__ pattern,
                   const char* const __restrict__ replacement,
                   char* const __restrict__ new_str) {
  RCSW_FPC_NV(NULL, NULL != original, NULL != pattern, NULL != replacement);
  size_t      orilen = stdio_strlen(original);
  size_t      replen = stdio_strlen(replacement);
  size_t      patlen = stdio_strlen(pattern);
  const char* oriptr;
  const char* patloc;
  size_t      patcnt = 0;

  /* An empty pattern matches everywhere without advancing: nothing to do */
  if (0 == patlen) {
    stdio_strcpy(new_str, original);
    return new_str;
  }

  /* find how many times the pattern occurs in the original string */
  for (oriptr = original; (patloc = stdio_strstr(oriptr, pattern));
       oriptr = patloc + patlen) {
    patcnt++;
  }

  /* allocate memory for the new string */
  size_t newlen   = orilen + (patcnt * (replen - patlen));
  new_str[newlen] = '\0';

  /* copy the original string, replacing all the instances of the pattern */
  char* retptr = new_str;
  for (oriptr = original; (patloc = stdio_strstr(oriptr, pattern));
       oriptr = patloc + patlen) {
    size_t skiplen = (size_t)(patloc - oriptr);

    /* copy the section until the occurrence of the pattern */
    stdio_strncpy(retptr, oriptr, skiplen);
    retptr += skiplen;

    /* copy the replacement */
    stdio_strncpy(retptr, replacement, replen);
    retptr += replen;
  }

  /* copy the rest of the string */
  stdio_strcpy(retptr, oriptr);
  return new_str;
}

void stdio_strrev(char* const s, size_t len) {
  if (len < 2) {
    return;
  }
  for (size_t i = 0, j = len - 1; i < j; ++i, --j) {
    char tmp = s[i];
    s[i]     = s[j];
    s[j]     = tmp;
  }
} /* stdio_strrev() */

size_t stdio_strlen(const char* const s) {
  RCSW_FPC_NV(0, NULL != s);

  char const* p;
  for (p = s; *p; p++) {
  }

  return (size_t)(p - s);
}

size_t stdio_strnlen(const char* const s, size_t maxsize) {
  RCSW_FPC_NV(0, NULL != s);

  char const* p;
  for (p = s; *p && maxsize--; p++) {
  }

  return (size_t)(p - s);
}

const char* stdio_strchr(const char* haystack, char needle) {
  if (NULL == haystack) {
    return NULL;
  }
  /* As with strchr(), the terminating NUL is part of the string */
  for (;; ++haystack) {
    if (*haystack == needle) {
      return haystack;
    }
    if ('\0' == *haystack) {
      return NULL;
    }
  } /* for(;;) */
}

const char* stdio_strstr(const char* const __restrict__ haystack,
                         const char* const __restrict__ needle) {
  const char* p1 = (const char*)haystack;
  if (!*needle) { /* null string */
    return p1;
  }

  while (*p1) { /* while there are chars left to check in haystack */
    const char* p1_curr = p1;
    const char* p2      = (const char*)needle;
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
}

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
}

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
  /* As with strcmp(), bytes compare as unsigned char */
  const unsigned char* t1 = (const unsigned char*)s1;
  const unsigned char* t2 = (const unsigned char*)s2;
  while (*t1 == *t2) {
    if (*t1 == '\0') {
      return 0;
    }
    t1++;
    t2++;
  }
  return (int)*t1 - (int)*t2;
}

int stdio_strncmp(const char* const s1, const char* const s2, size_t len) {
  /* As with strncmp(), bytes compare as unsigned char */
  const unsigned char* t1 = (const unsigned char*)s1;
  const unsigned char* t2 = (const unsigned char*)s2;

  for (size_t i = 0; i < len; ++i) {
    if (t1[i] != t2[i]) {
      return (int)t1[i] - (int)t2[i];
    }
    if ('\0' == t1[i]) {
      return 0;
    }
  } /* for(i..) */
  return 0;
}

int stdio_tolower(int c) {
  if (c >= 'A' && c <= 'Z') {
    c += ('a' - 'A');
  }
  return c;
}

int stdio_toupper(int c) {
  if (c >= 'a' && c <= 'z') {
    c += ('A' - 'a');
  }
  return c;
}

void* stdio_memcpy(void* const __restrict__ dest,
                   const void* const __restrict__ src,
                   size_t n) {
  char*       d = dest;
  const char* s = src;
  for (size_t i = 0; i < n; i++) {
    d[i] = s[i];
  }
  return dest;
}

void* stdio_memset(void* const __restrict__ dest, int c, size_t n) {
  char* d = dest;
  for (size_t i = 0; i < n; i++) {
    d[i] = (char)c;
  }
  return dest;
}

END_C_DECLS
