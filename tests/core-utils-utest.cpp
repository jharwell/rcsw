/**
 * \file core-utils-utest.cpp
 *
 * Unit tests for core utility macros: RCSW_MIN, RCSW_MAX, RCSW_CLAMP,
 * RCSW_IS_ODD/EVEN, RCSW_IS_BETWEEN*, RCSW_ARRAY_ELTS, RCSW_CONTAINER_OF,
 * RCSW_FIELD_SIZEOF, RCSW_CHECK, RCSW_CHECK_PTR.
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#define CATCH_CONFIG_PREFIX_ALL
#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <climits>

#include "rcsw/core/core.h"

/*******************************************************************************
 * RCSW_MIN / RCSW_MAX / RCSW_MIN3 / RCSW_MAX3
 ******************************************************************************/
CATCH_TEST_CASE("RCSW_MIN - basic", "[core][utils]") {
  CATCH_REQUIRE(RCSW_MIN(3, 5) == 3);
  CATCH_REQUIRE(RCSW_MIN(5, 3) == 3);
  CATCH_REQUIRE(RCSW_MIN(4, 4) == 4);
  CATCH_REQUIRE(RCSW_MIN(-1, 1) == -1);
}

CATCH_TEST_CASE("RCSW_MIN - no double evaluation", "[core][utils]") {
  int x = 3;
  int y = 5;
  int r = RCSW_MIN(x++, y++);
  /* each argument evaluated exactly once */
  CATCH_REQUIRE(r == 3);
  CATCH_REQUIRE(x == 4);
  CATCH_REQUIRE(y == 6);
}

CATCH_TEST_CASE("RCSW_MAX - basic", "[core][utils]") {
  CATCH_REQUIRE(RCSW_MAX(3, 5) == 5);
  CATCH_REQUIRE(RCSW_MAX(5, 3) == 5);
  CATCH_REQUIRE(RCSW_MAX(4, 4) == 4);
  CATCH_REQUIRE(RCSW_MAX(-1, 1) == 1);
}

CATCH_TEST_CASE("RCSW_MAX - no double evaluation", "[core][utils]") {
  int x = 3;
  int y = 5;
  int r = RCSW_MAX(x++, y++);
  CATCH_REQUIRE(r == 5);
  CATCH_REQUIRE(x == 4);
  CATCH_REQUIRE(y == 6);
}

CATCH_TEST_CASE("RCSW_MIN3", "[core][utils]") {
  CATCH_REQUIRE(RCSW_MIN3(1, 2, 3) == 1);
  CATCH_REQUIRE(RCSW_MIN3(3, 1, 2) == 1);
  CATCH_REQUIRE(RCSW_MIN3(3, 2, 1) == 1);
  CATCH_REQUIRE(RCSW_MIN3(5, 5, 5) == 5);
}

CATCH_TEST_CASE("RCSW_MAX3", "[core][utils]") {
  CATCH_REQUIRE(RCSW_MAX3(1, 2, 3) == 3);
  CATCH_REQUIRE(RCSW_MAX3(3, 1, 2) == 3);
  CATCH_REQUIRE(RCSW_MAX3(3, 2, 1) == 3);
  CATCH_REQUIRE(RCSW_MAX3(5, 5, 5) == 5);
}

/*******************************************************************************
 * RCSW_CLAMP
 ******************************************************************************/
CATCH_TEST_CASE("RCSW_CLAMP - within range", "[core][utils]") {
  CATCH_REQUIRE(RCSW_CLAMP(5, 0, 10) == 5);
}

CATCH_TEST_CASE("RCSW_CLAMP - below min", "[core][utils]") {
  CATCH_REQUIRE(RCSW_CLAMP(-5, 0, 10) == 0);
}

CATCH_TEST_CASE("RCSW_CLAMP - above max", "[core][utils]") {
  CATCH_REQUIRE(RCSW_CLAMP(15, 0, 10) == 10);
}

CATCH_TEST_CASE("RCSW_CLAMP - at boundaries", "[core][utils]") {
  CATCH_REQUIRE(RCSW_CLAMP(0, 0, 10) == 0);
  CATCH_REQUIRE(RCSW_CLAMP(10, 0, 10) == 10);
}

CATCH_TEST_CASE("RCSW_CLAMP - no double evaluation", "[core][utils]") {
  int v = 15, lo = 0, hi = 10;
  int r = RCSW_CLAMP(v++, lo++, hi++);
  CATCH_REQUIRE(r == 10);
  CATCH_REQUIRE(v == 16);
  CATCH_REQUIRE(lo == 1);
  CATCH_REQUIRE(hi == 11);
}

/*******************************************************************************
 * RCSW_IS_ODD / RCSW_IS_EVEN
 ******************************************************************************/
CATCH_TEST_CASE("RCSW_IS_ODD / RCSW_IS_EVEN", "[core][utils]") {
  CATCH_REQUIRE(RCSW_IS_ODD(1));
  CATCH_REQUIRE(RCSW_IS_ODD(3));
  CATCH_REQUIRE(RCSW_IS_ODD(INT_MAX));   /* INT_MAX is odd */
  CATCH_REQUIRE(!RCSW_IS_ODD(0));
  CATCH_REQUIRE(!RCSW_IS_ODD(4));

  CATCH_REQUIRE(RCSW_IS_EVEN(0));
  CATCH_REQUIRE(RCSW_IS_EVEN(4));
  CATCH_REQUIRE(!RCSW_IS_EVEN(3));
}

/*******************************************************************************
 * RCSW_IS_BETWEENC / O / HO
 ******************************************************************************/
CATCH_TEST_CASE("RCSW_IS_BETWEENC - closed interval [lo, hi]",
                "[core][utils]") {
  CATCH_REQUIRE(RCSW_IS_BETWEENC(5, 0, 10));
  CATCH_REQUIRE(RCSW_IS_BETWEENC(0, 0, 10));   /* at lower bound */
  CATCH_REQUIRE(RCSW_IS_BETWEENC(10, 0, 10));  /* at upper bound */
  CATCH_REQUIRE(!RCSW_IS_BETWEENC(-1, 0, 10));
  CATCH_REQUIRE(!RCSW_IS_BETWEENC(11, 0, 10));
}

CATCH_TEST_CASE("RCSW_IS_BETWEENO - open interval (lo, hi)",
                "[core][utils]") {
  CATCH_REQUIRE(RCSW_IS_BETWEENO(5, 0, 10));
  CATCH_REQUIRE(!RCSW_IS_BETWEENO(0, 0, 10));   /* lower bound excluded */
  CATCH_REQUIRE(!RCSW_IS_BETWEENO(10, 0, 10));  /* upper bound excluded */
  CATCH_REQUIRE(!RCSW_IS_BETWEENO(-1, 0, 10));
  CATCH_REQUIRE(!RCSW_IS_BETWEENO(11, 0, 10));
}

CATCH_TEST_CASE("RCSW_IS_BETWEENHO - half-open interval [lo, hi)",
                "[core][utils]") {
  CATCH_REQUIRE(RCSW_IS_BETWEENHO(0, 0, 10));   /* lower bound included */
  CATCH_REQUIRE(RCSW_IS_BETWEENHO(5, 0, 10));
  CATCH_REQUIRE(!RCSW_IS_BETWEENHO(10, 0, 10)); /* upper bound excluded */
  CATCH_REQUIRE(!RCSW_IS_BETWEENHO(-1, 0, 10));
  CATCH_REQUIRE(!RCSW_IS_BETWEENHO(11, 0, 10));
}

/*******************************************************************************
 * RCSW_ARRAY_ELTS
 ******************************************************************************/
CATCH_TEST_CASE("RCSW_ARRAY_ELTS", "[core][utils]") {
  int    a[5]  = {0};
  double b[10] = {0};
  char   c[1]  = {0};
  CATCH_REQUIRE(RCSW_ARRAY_ELTS(a) == 5);
  CATCH_REQUIRE(RCSW_ARRAY_ELTS(b) == 10);
  CATCH_REQUIRE(RCSW_ARRAY_ELTS(c) == 1);
}

/*******************************************************************************
 * RCSW_CONTAINER_OF
 ******************************************************************************/
CATCH_TEST_CASE("RCSW_CONTAINER_OF", "[core][utils]") {
  struct outer {
    int   x;
    float y;
    char  z;
  };
  struct outer obj{42, 3.14f, 'A'};

  float* yp  = &obj.y;
  auto*  got = RCSW_CONTAINER_OF(yp, struct outer, y);
  CATCH_REQUIRE(got == &obj);
  CATCH_REQUIRE(got->x == 42);
  CATCH_REQUIRE(got->z == 'A');
}

/*******************************************************************************
 * RCSW_FIELD_SIZEOF
 ******************************************************************************/
CATCH_TEST_CASE("RCSW_FIELD_SIZEOF", "[core][utils]") {
  struct s {
    uint8_t  a;
    uint32_t b;
    uint64_t c;
  };
  CATCH_REQUIRE(RCSW_FIELD_SIZEOF(struct s, a) == sizeof(uint8_t));
  CATCH_REQUIRE(RCSW_FIELD_SIZEOF(struct s, b) == sizeof(uint32_t));
  CATCH_REQUIRE(RCSW_FIELD_SIZEOF(struct s, c) == sizeof(uint64_t));
}

/*******************************************************************************
 * RCSW_CHECK / RCSW_CHECK_PTR
 ******************************************************************************/
static int check_test(int x) {
  RCSW_CHECK(x > 0);
  return x * 2;
error:
  return -1;
}

static int check_ptr_test(void* p) {
  RCSW_CHECK_PTR(p);
  return 1;
error:
  return -1;
}

CATCH_TEST_CASE("RCSW_CHECK - passes", "[core][utils]") {
  CATCH_REQUIRE(check_test(5) == 10);
}

CATCH_TEST_CASE("RCSW_CHECK - fails jumps to error", "[core][utils]") {
  CATCH_REQUIRE(check_test(-1) == -1);
}

CATCH_TEST_CASE("RCSW_CHECK_PTR - non-null passes", "[core][utils]") {
  int x = 0;
  CATCH_REQUIRE(check_ptr_test(&x) == 1);
}

CATCH_TEST_CASE("RCSW_CHECK_PTR - null fails", "[core][utils]") {
  CATCH_REQUIRE(check_ptr_test(nullptr) == -1);
}

/*******************************************************************************
 * Constants
 ******************************************************************************/
CATCH_TEST_CASE("Core constants", "[core][utils]") {
  CATCH_REQUIRE(RCSW_E9 == 1000000000);
  CATCH_REQUIRE(RCSW_E6 == 1000000);
  CATCH_REQUIRE(RCSW_E3 == 1000);
}

/*******************************************************************************
 * RCSW_XSTR / RCSW_JOIN
 ******************************************************************************/
CATCH_TEST_CASE("RCSW_XSTR stringification", "[core][utils]") {
  /* stringification of a numeric literal */
  const char* s = RCSW_XSTR(42);
  CATCH_REQUIRE(std::string(s) == "42");
}

CATCH_TEST_CASE("RCSW_JOIN token pasting", "[core][utils]") {
  /* verify the pasted token names a valid identifier */
#define MY_PREFIX_ hello
#define MY_SUFFIX  world
  /* if pasting failed this would be a compile error */
  int RCSW_JOIN(MY_PREFIX_, MY_SUFFIX) = 99;
  CATCH_REQUIRE(RCSW_JOIN(MY_PREFIX_, MY_SUFFIX) == 99);
#undef MY_PREFIX_
#undef MY_SUFFIX
}
