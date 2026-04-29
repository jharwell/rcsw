/**
 * \file alg-lcs-utest.cpp
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#define CATCH_CONFIG_PREFIX_ALL
#include <catch2/catch_test_macros.hpp>

#include <cstring>

#include "rcsw/algorithm/lcs.h"

/*******************************************************************************
 * Test Helpers
 ******************************************************************************/
/**
 * Run both lcs_iter and lcs_rec on fresh lcs_calculator instances (to ensure
 * independence), verify they agree, and check the expected length and sequence.
 */
static void check_lcs(const char* x,
                      const char* y,
                      size_t      expected_len,
                      const char* expected_seq) {
  /* --- iterative --- */
  struct lcs_calculator lcs_i;
  CATCH_REQUIRE(OK == lcs_init(&lcs_i, x, y));
  CATCH_REQUIRE(OK == lcs_iter(&lcs_i));
  CATCH_REQUIRE(lcs_i.size == expected_len);
  if (expected_seq != nullptr) {
    CATCH_REQUIRE(strcmp(lcs_i.sequence, expected_seq) == 0);
  }

  /* --- recursive (fresh instance — does not share state with iter) --- */
  struct lcs_calculator lcs_r;
  CATCH_REQUIRE(OK == lcs_init(&lcs_r, x, y));
  int rec_len = lcs_rec(&lcs_r);
  CATCH_REQUIRE(rec_len == (int)expected_len);

  /* both agree */
  CATCH_REQUIRE((size_t)rec_len == lcs_i.size);

  lcs_destroy(&lcs_i);
  lcs_destroy(&lcs_r);
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("LCS - NULL Input Rejected", "[alg][lcs]") {
  char x[] = "ABC";
  char y[] = "AC";
  struct lcs_calculator lcs;
  CATCH_REQUIRE(ERROR == lcs_init(nullptr, x, y));
  CATCH_REQUIRE(ERROR == lcs_init(&lcs, nullptr, y));
  CATCH_REQUIRE(ERROR == lcs_init(&lcs, x, nullptr));
  lcs_destroy(nullptr);  /* must not crash */
}

CATCH_TEST_CASE("LCS - Known Cases", "[alg][lcs]") {
  CATCH_SECTION("ABCRCQ7Xz vs o15RCQ0Xz") {
    check_lcs("ABCRCq7Xz", "o15RCQ0Xz", 4, nullptr);
    /* length-5 original test from the old test file */
    check_lcs("ABCRCQ7Xz", "o15RCQ0Xz", 5, "RCQXz");
  }
  CATCH_SECTION("ABCDGH vs AEDFHR") {
    check_lcs("ABCDGH", "AEDFHR", 3, "ADH");
  }
  CATCH_SECTION("AGGTAB vs GXTXAYB") {
    check_lcs("AGGTAB", "GXTXAYB", 4, nullptr);
  }
}

CATCH_TEST_CASE("LCS - Equal Strings", "[alg][lcs]") {
  /* LCS of a string with itself is the string itself */
  const char* s = "HELLO";
  check_lcs(s, s, strlen(s), "HELLO");
}

CATCH_TEST_CASE("LCS - No Common Characters", "[alg][lcs]") {
  check_lcs("AAAA", "BBBB", 0, "");
}

CATCH_TEST_CASE("LCS - One Empty String", "[alg][lcs]") {
  check_lcs("", "ABCD", 0, "");
  check_lcs("ABCD", "", 0, "");
}

CATCH_TEST_CASE("LCS - Single Character Strings", "[alg][lcs]") {
  check_lcs("A", "A", 1, "A");
  check_lcs("A", "B", 0, "");
}

CATCH_TEST_CASE("LCS - One String is Subsequence of Other", "[alg][lcs]") {
  /* "ACE" is a subsequence of "ABCDE" */
  check_lcs("ACE", "ABCDE", 3, "ACE");
}

CATCH_TEST_CASE("LCS - Repeated Characters", "[alg][lcs]") {
  check_lcs("AABAA", "AABAA", 5, "AABAA");
  check_lcs("AAAA", "AA", 2, "AA");
}
