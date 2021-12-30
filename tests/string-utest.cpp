/**
 * @file string-test.cpp
 *
 * Test of string part of stdio library.
 *
 * @copyright 2017 John Harwell, All rights reserved.
*/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#if !defined(__nos__)
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#endif
#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_PREFIX_ALL
#include <catch.hpp>

#include "rcsw/stdio/sstdio.h"
#include "rcsw/stdio/sstring.h"
#include "rcsw/common/dbg.h"
#include "rcsw/utils/utils.h"

/******************************************************************************
 * Constant Definitions
 *****************************************************************************/
#define NUM_TESTS        7
#define MAX_STRING_SIZE 100

/*******************************************************************************
 * Test Function Forward Declarations
 ******************************************************************************/
static void strlen_test(void);
static void strrev_test(void);
static void strchr_test(void);
static void strcpy_test(void);
static void strncpy_test(void);
static void strcmp_test(void);
static void strncmp_test(void);
static void strrep_test(void);

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("strlen() Test [string]") { strlen_test(); }
CATCH_TEST_CASE("strrev() Test [string]") { strrev_test(); }
CATCH_TEST_CASE("strchr() Test [string]") { strchr_test(); }
CATCH_TEST_CASE("strcpy() Test [string]") { strcpy_test(); }
CATCH_TEST_CASE("strncpy() Test [string]") { strncpy_test(); }
CATCH_TEST_CASE("strcmp() Test [string]") { strcmp_test(); }
CATCH_TEST_CASE("strncmp() Test [string]") { strncmp_test(); }
CATCH_TEST_CASE("strrep() Test [string]") { strrep_test(); }

static void strlen_test(void) {
  char s[MAX_STRING_SIZE];
  int i;
  int len1, len2;
  for (i = 1; i < MAX_STRING_SIZE; i++) {
    string_gen(s, i);

    len1 = sstring_strlen(s);
    len2 = strlen(s);
    CATCH_REQUIRE(len1 == i -1);
    CATCH_REQUIRE(len1 == len2);
  }
} /* strlen_test() */

static void strrev_test(void) {
  char s1[MAX_STRING_SIZE];
  char s2[MAX_STRING_SIZE];
  int i, j;
  for (i = 1; i < MAX_STRING_SIZE; i++) {
    string_gen(s1, i);
    memcpy(s2, s1, i);
    sstring_strrev(s1, i);
    for (j = 0; j < i; j++) {
      CATCH_REQUIRE(s1[j] == s2[i - j -1]);
    }
  }
} /* strrev_test() */

static void strchr_test(void) {
  char s1[MAX_STRING_SIZE];
  int i, j;
  char* rval1;
  const char* rval2;

  for (i = 1; i < MAX_STRING_SIZE; i++) {
    string_gen(s1, i);
    for (j = 0; j < i; j++) {
      int c = rand() % (126 - 33 + 1) + 33;
      rval1 = strchr(s1, c);
      rval2 = sstring_strchr(s1, c);

      CATCH_REQUIRE(rval1 == rval2);
    }
  }
} /* strchr_test() */

static void strcpy_test(void) {
  char s1[MAX_STRING_SIZE];
  char s2[MAX_STRING_SIZE];
  int i;

  for (i = 1; i < MAX_STRING_SIZE; i++) {
    string_gen(s1, i);
    memset(s2, 0, MAX_STRING_SIZE);
    sstring_strcpy(s2, s1);
    CATCH_REQUIRE(strncmp(s1, s2, i) == 0);
  }
} /* strcpy_test() */

static void strncpy_test(void) {
  char s1[MAX_STRING_SIZE];
  char s2[MAX_STRING_SIZE];
  int i, j;

  for (i = 1; i < MAX_STRING_SIZE; i++) {
    string_gen(s1, i);
    for (j = 0; j < i; j++) {
      memset(s2, 0, MAX_STRING_SIZE);
      sstring_strncpy(s2, s1, j);
      CATCH_REQUIRE(strncmp(s1, s2, j) == 0);
    } /* for(j..) */
  } /* for(i..) */
} /* strncpy_test() */

static void strcmp_test(void) {
  char s1[MAX_STRING_SIZE];
  char s2[MAX_STRING_SIZE];
  int i;
  int rval1, rval2;

  /* test on random strings */
  for (i = 1; i < MAX_STRING_SIZE; i++) {
    string_gen(s1, i);
    string_gen(s2, i);
    rval1 = sstring_strcmp(s1, s2);
    rval2 = strcmp(s1, s2);
    CATCH_REQUIRE(!(((rval1 < 0) && (rval2 >= 0))||
                    ((rval1 > 0) && (rval2 <= 0)) ||
                    ((rval1 == 0) && (rval2 != 0))||
                    ((rval2 == 0) && (rval1 != 0))));
  } /* for() */

  /* test on strings that are known to match */
  for (i = 1; i < MAX_STRING_SIZE; i++) {
    string_gen(s1, i);
    memcpy(s2, s1, i);
    CATCH_REQUIRE(sstring_strcmp(s1, s2) == 0);
  }
} /* strcmp_test() */

static void strncmp_test(void) {
  char s1[MAX_STRING_SIZE];
  char s2[MAX_STRING_SIZE];
  int i, j;
  int rval1, rval2;

  /* test on random strings */
  for (i = 1; i < MAX_STRING_SIZE; i++) {
    string_gen(s1, i);
    string_gen(s2, i);
    for (j = 0; j < i; j++) {
      rval1 = sstring_strncmp(s1, s2, j);
      rval2 = strncmp(s1, s2, j);
      CATCH_REQUIRE(!(((rval1 < 0) && (rval2 >= 0)) &&
                      ((rval1 > 0) && (rval2 <= 0)) &&
                      ((rval1 == 0) && (rval2 != 0)) &&
                      ((rval2 == 0) && (rval1 != 0))));
    }
  }

  /* test on strings that are known to match */
  for (i = 1; i < MAX_STRING_SIZE; i++) {
    string_gen(s1, i);
    for (j = 0; j < i; j++) {
      memcpy(s2, s1, i);
      CATCH_REQUIRE(sstring_strncmp(s1, s2, j) == 0);
    }
  }
} /* strncmp_test() */

static void strrep_test(void) {
  char original[MAX_STRING_SIZE];
  char pattern[MAX_STRING_SIZE];
  char replacement[MAX_STRING_SIZE];
  char new_str[MAX_STRING_SIZE* MAX_STRING_SIZE];
  int i, j, k;
  char *tmp;
  int pat_count = 0;
  int pat_len = 2;
  int rep_len = 2;

  /* test on random strings */
  for (i = 1; i < MAX_STRING_SIZE; i++) {
    if (((i % (MAX_STRING_SIZE/10)) == 0) && (i != 0)) {
      fflush(NULL);
    }
    /* generate test string */
    string_gen(original, i);
    rep_len = 2;

    for (j = 1; j < MAX_STRING_SIZE; j++) {
      /* generate string pattern to look for */
      string_gen(pattern, pat_len);

      /* copy pattern to a few random locations within original */
      for (k = 0; k < rand() % 10; k++) {
        int pos = rand() % 10;
        if (pos + strlen(pattern) < strlen(original)) {
          memcpy(original+pos, pattern, strlen(pattern));
        }
      }

      /* figure out how many times replacement pattern occurs in test string */
      tmp = original;
      pat_count = 0;
      char* tmp2;
      for (tmp = original; (tmp2=strstr(tmp, pattern)); tmp = tmp2 + 1) {
        pat_count++;
      }


      /* generate replacement string */
      string_gen(replacement, rep_len);

      /* replace the all occurrences of pattern in original with replacement */
      /* printf("original: %s pattern: %s replacement: %s\n", original, pattern, replacement); */
      /* printf("new: %s\n",new); */
      sstring_strrep(original, pattern, replacement, new_str);
      CATCH_REQUIRE(strlen(original) + pat_count * strlen(replacement) -
                    pat_count * strlen(pattern) == strlen(new_str));
      rep_len++;
    } /* for (j...) */
  } /* for (i...) */
  printf("\n");
} /* strrep_test() */
