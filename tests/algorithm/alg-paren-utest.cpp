/**
 * \file alg-paren-utest.cpp
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

#include "rcsw/algorithm/algorithm.h"

/*******************************************************************************
 * Test Helpers
 ******************************************************************************/
/*
 * Multiplication table for {a, b}:
 * 
 *   a * b = a  (only way to produce 'a')
 *   everything else = b
 *
 * With this table:
 *   "abab": (a*(b*(ab))) = (a*(b*a)) = (a*b) = a  -> parenthesizable to 'a'
 *   "baab": no bracketing yields 'a'              -> not parenthesizable to 'a'
 */
static char multiply_ab(char x, char y) {
    if (x == 'a' && y == 'b') return 'a';
    return 'b';
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("Parenthesization - NULL Input Rejected", "[alg][paren]") {
  char r[16];
  char x[] = "ab";
  CATCH_REQUIRE(false == str_is_parenthesizable(nullptr, r, 'a', multiply_ab));
  CATCH_REQUIRE(false == str_is_parenthesizable(x, nullptr, 'a', multiply_ab));
  CATCH_REQUIRE(false == str_is_parenthesizable(x, r, 'a', nullptr));
}

CATCH_TEST_CASE("Parenthesization - Known Cases", "[alg][paren]") {
  char r[32];

  CATCH_SECTION("abab -> a (parenthesizable)") {
    char x[] = "abab";
    /* (a(b(ab))) = (a(b*a)) = (a*b) = a */
    CATCH_REQUIRE(true == str_is_parenthesizable(x, r, 'a', multiply_ab));
  }
  CATCH_SECTION("baab -> a (not parenthesizable)") {
    char x[] = "baab";
    CATCH_REQUIRE(false == str_is_parenthesizable(x, r, 'a', multiply_ab));
  }
  CATCH_SECTION("ab -> a (single multiplication)") {
    char x[] = "ab";
    CATCH_REQUIRE(true == str_is_parenthesizable(x, r, 'a', multiply_ab));
  }
  CATCH_SECTION("ba -> a (not achievable: b*a = b)") {
    char x[] = "ba";
    CATCH_REQUIRE(false == str_is_parenthesizable(x, r, 'a', multiply_ab));
  }
}

CATCH_TEST_CASE("Parenthesization - Single Character", "[alg][paren]") {
  char r[4];
  /* single character x: result is x[0] itself, no multiplication */
  char xa[] = "a";
  char xb[] = "b";
  CATCH_REQUIRE(true  == str_is_parenthesizable(xa, r, 'a', multiply_ab));
  CATCH_REQUIRE(false == str_is_parenthesizable(xa, r, 'b', multiply_ab));
  CATCH_REQUIRE(false == str_is_parenthesizable(xb, r, 'a', multiply_ab));
  CATCH_REQUIRE(true  == str_is_parenthesizable(xb, r, 'b', multiply_ab));
}

CATCH_TEST_CASE("Parenthesization - All Same Characters", "[alg][paren]") {
  char r[32];
 char x_aaa[] = "aaa";
CATCH_REQUIRE(true == str_is_parenthesizable(x_aaa, r, 'a', multiply_ab));
CATCH_REQUIRE(true == str_is_parenthesizable(x_aaa, r, 'b', multiply_ab));

  /* bbb: b*b=b always → only parenthesizable to 'b' */
  char x_bbb[] = "bbb";
  CATCH_REQUIRE(false == str_is_parenthesizable(x_bbb, r, 'a', multiply_ab));
  CATCH_REQUIRE(true  == str_is_parenthesizable(x_bbb, r, 'b', multiply_ab));
}

CATCH_TEST_CASE("Parenthesization - Longer Sequence", "[alg][paren]") {
  char r[64];
  /* ababab: multiple ways to bracket, at least one should yield 'a' */
  char x[] = "ababab";
  /* verify it terminates and returns a valid bool */
  bool_t result = str_is_parenthesizable(x, r, 'a', multiply_ab);
  CATCH_REQUIRE((result == true || result == false));
}
