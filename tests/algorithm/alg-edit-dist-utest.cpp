/**
 * \file alg-edit-dist-utest.cpp
 *
 * Unit tests for the edit distance algorithm.
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

#include "rcsw/algorithm/edit_dist.h"

/*******************************************************************************
 * Test Helpers
 ******************************************************************************/
static bool_t char_cmpe(const void* const e1, const void* const e2) {
  return (bool_t)(*((const char*)e1) == *((const char*)e2));
}

/**
 * Run both iterative and recursive edit_dist_find on fresh finders, verify
 * they agree and match the expected distance.
 */
static void check_edit_dist(const char* a,
                             const char* b,
                             int         expected) {
  /* iterative */
  struct edit_dist_finder fi;
  CATCH_REQUIRE(OK == edit_dist_init(&fi,
                                     a,
                                     b,
                                     sizeof(char),
                                     char_cmpe,
                                     (size_t(*)(const void*))strlen));
  int dist_iter = edit_dist_find(&fi, ekEXEC_ITER);
  CATCH_REQUIRE(dist_iter == expected);
  edit_dist_destroy(&fi);

  /* recursive */
  struct edit_dist_finder fr;
  CATCH_REQUIRE(OK == edit_dist_init(&fr,
                                     a,
                                     b,
                                     sizeof(char),
                                     char_cmpe,
                                     (size_t(*)(const void*))strlen));
  int dist_rec = edit_dist_find(&fr, ekEXEC_REC);
  CATCH_REQUIRE(dist_rec == expected);
  edit_dist_destroy(&fr);

  /* both agree */
  CATCH_REQUIRE(dist_iter == dist_rec);
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("Edit Distance - NULL Input Rejected", "[alg][edit_dist]") {
  char a[] = "ABC";
  char b[] = "AC";
  struct edit_dist_finder f;
  CATCH_REQUIRE(ERROR == edit_dist_init(nullptr, a, b, sizeof(char),
                                        char_cmpe,
                                        (size_t(*)(const void*))strlen));
  CATCH_REQUIRE(ERROR == edit_dist_init(&f, nullptr, b, sizeof(char),
                                        char_cmpe,
                                        (size_t(*)(const void*))strlen));
  CATCH_REQUIRE(ERROR == edit_dist_init(&f, a, nullptr, sizeof(char),
                                        char_cmpe,
                                        (size_t(*)(const void*))strlen));
  edit_dist_destroy(nullptr);  /* must not crash */
}

CATCH_TEST_CASE("Edit Distance - Equal Strings", "[alg][edit_dist]") {
  /* distance from a string to itself is 0 */
  check_edit_dist("ABCDE", "ABCDE", 0);
  check_edit_dist("A", "A", 0);
}

CATCH_TEST_CASE("Edit Distance - One Empty String", "[alg][edit_dist]") {
  /* converting empty -> "ABC" requires 3 insertions */
  check_edit_dist("", "ABC", 3);
  /* converting "ABC" -> empty requires 3 deletions */
  check_edit_dist("ABC", "", 3);
  check_edit_dist("", "", 0);
}

CATCH_TEST_CASE("Edit Distance - Single Character", "[alg][edit_dist]") {
  check_edit_dist("A", "B", 1);  /* 1 substitution */
  check_edit_dist("A", "AB", 1); /* 1 insertion */
  check_edit_dist("AB", "A", 1); /* 1 deletion */
}

CATCH_TEST_CASE("Edit Distance - Known Cases", "[alg][edit_dist]") {
  CATCH_SECTION("ABCRcQ7Xz vs o15RCQ0Xz") {
    check_edit_dist("ABCRcQ7Xz", "o15RCQ0Xz", 5);
  }
  CATCH_SECTION("kitten vs sitting") {
    /* classic example: 3 operations */
    check_edit_dist("kitten", "sitting", 3);
  }
  CATCH_SECTION("saturday vs sunday") {
    check_edit_dist("saturday", "sunday", 3);
  }
  CATCH_SECTION("completely different") {
    /* no common characters: distance = max(len_a, len_b) */
    check_edit_dist("AAAA", "BBBB", 4);
  }
}

CATCH_TEST_CASE("Edit Distance - Prefix/Suffix", "[alg][edit_dist]") {
  /* b is a prefix of a — only deletions needed */
  check_edit_dist("ABCDE", "ABC", 2);
  /* a is a suffix of b — only insertions needed */
  check_edit_dist("CDE", "ABCDE", 2);
}

CATCH_TEST_CASE("Edit Distance - Symmetry", "[alg][edit_dist]") {
  /* edit_dist(a, b) == edit_dist(b, a) */
  check_edit_dist("ABCDE", "ACE", 2);
  check_edit_dist("ACE", "ABCDE", 2);
}
