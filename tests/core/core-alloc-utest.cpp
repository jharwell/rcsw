/**
 * \file core-alloc-utest.cpp
 *
 * Unit tests for rcsw_alloc() and rcsw_free().
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

#include <cstring>

#include "rcsw/core/alloc.h"
#include "rcsw/core/flags.h"

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("rcsw_alloc - malloc path", "[core][alloc]") {
  void* p = rcsw_alloc(nullptr, 64, RCSW_NONE);
  CATCH_REQUIRE(p != nullptr);
  rcsw_free(p, RCSW_NONE);
}

CATCH_TEST_CASE("rcsw_alloc - NOALLOC_HANDLE uses caller pointer",
                "[core][alloc]") {
  uint8_t buf[64];
  void*   p = rcsw_alloc(buf, sizeof(buf), RCSW_NOALLOC_HANDLE);
  CATCH_REQUIRE(p == buf);
  rcsw_free(p, RCSW_NOALLOC_HANDLE);
}

CATCH_TEST_CASE("rcsw_alloc - NOALLOC_DATA uses caller pointer",
                "[core][alloc]") {
  uint8_t buf[64];
  void*   p = rcsw_alloc(buf, sizeof(buf), RCSW_NOALLOC_DATA);
  CATCH_REQUIRE(p == buf);
  rcsw_free(p, RCSW_NOALLOC_DATA);
}

CATCH_TEST_CASE("rcsw_alloc - NOALLOC_META uses caller pointer",
                "[core][alloc]") {
  uint8_t buf[64];
  void*   p = rcsw_alloc(buf, sizeof(buf), RCSW_NOALLOC_META);
  CATCH_REQUIRE(p == buf);
  rcsw_free(p, RCSW_NOALLOC_META);
}

CATCH_TEST_CASE("rcsw_alloc - ZALLOC zeroes memory", "[core][alloc]") {
  void* p = rcsw_alloc(nullptr, 64, RCSW_ZALLOC);
  CATCH_REQUIRE(p != nullptr);
  uint8_t expected[64];
  memset(expected, 0, sizeof(expected));
  CATCH_REQUIRE(memcmp(p, expected, 64) == 0);
  rcsw_free(p, RCSW_NONE);
}

CATCH_TEST_CASE("rcsw_alloc - NOALLOC|ZALLOC zeroes caller memory",
                "[core][alloc]") {
  uint8_t buf[64];
  memset(buf, 0xFF, sizeof(buf));
  void* p = rcsw_alloc(buf, sizeof(buf), RCSW_NOALLOC_HANDLE | RCSW_ZALLOC);
  CATCH_REQUIRE(p == buf);
  uint8_t expected[64];
  memset(expected, 0, sizeof(expected));
  CATCH_REQUIRE(memcmp(p, expected, 64) == 0);
}

CATCH_TEST_CASE("rcsw_free - NULL pointer is safe", "[core][alloc]") {
  rcsw_free(nullptr, RCSW_NONE);
  rcsw_free(nullptr, RCSW_NOALLOC_HANDLE);
}

CATCH_TEST_CASE("rcsw_free - NOALLOC flags skip free", "[core][alloc]") {
  /*
   * Stack address with NOALLOC flag: if rcsw_free() incorrectly calls free()
   * on it the heap allocator aborts. Completing without crash proves the
   * NOALLOC path is taken.
   */
  uint8_t stack_buf[64];
  rcsw_free(stack_buf, RCSW_NOALLOC_HANDLE);
  rcsw_free(stack_buf, RCSW_NOALLOC_DATA);
  rcsw_free(stack_buf, RCSW_NOALLOC_META);
  rcsw_free(stack_buf, RCSW_NOALLOC_ALL);
}

CATCH_TEST_CASE("rcsw_alloc - multiple independent allocations",
                "[core][alloc]") {
  const size_t N = 16;
  void*        ptrs[N];
  for (size_t i = 0; i < N; ++i) {
    ptrs[i] = rcsw_alloc(nullptr, 128, RCSW_NONE);
    CATCH_REQUIRE(ptrs[i] != nullptr);
    for (size_t j = 0; j < i; ++j) {
      CATCH_REQUIRE(ptrs[i] != ptrs[j]);
    }
  }
  for (size_t i = 0; i < N; ++i) {
    rcsw_free(ptrs[i], RCSW_NONE);
  }
}
