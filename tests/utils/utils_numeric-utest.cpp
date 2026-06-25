/**
 * \file utils_numeric-utest.cpp
 *
 * Unit tests for rcsw/utils/numeric.h: utils_permute(), utils_zchk(), and
 * utils_clamp_f255().
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
#include <cmath>

#include "rcsw/utils/numeric.h"
#include "rcsw/core/core.h"

/*******************************************************************************
 * Callbacks
 ******************************************************************************/

/* Count how many times utils_permute() fires the callback. */
static size_t g_permute_count = 0;

static void permute_counter_cb(void*) {
  ++g_permute_count;
}

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/

static void permute_count_test() {
  /* 4! = 24 permutations */
  uint32_t arr4[4] = {0x1, 0x2, 0x3, 0x4};
  g_permute_count = 0;
  utils_permute(arr4, RCSW_ARRAY_ELTS(arr4), sizeof(uint32_t), 0,
                permute_counter_cb);
  CATCH_REQUIRE(g_permute_count == 24u);

  /* 3! = 6 permutations */
  uint32_t arr3[3] = {0xA, 0xB, 0xC};
  g_permute_count = 0;
  utils_permute(arr3, RCSW_ARRAY_ELTS(arr3), sizeof(uint32_t), 0,
                permute_counter_cb);
  CATCH_REQUIRE(g_permute_count == 6u);

  /* 1! = 1 permutation */
  uint32_t arr1[1] = {0xFF};
  g_permute_count = 0;
  utils_permute(arr1, RCSW_ARRAY_ELTS(arr1), sizeof(uint32_t), 0,
                permute_counter_cb);
  CATCH_REQUIRE(g_permute_count == 1u);
}

static void permute_restores_test() {
  /*
   * utils_permute() must restore the array to its original order after all
   * permutations have been enumerated (it swaps in-place and restores).
   */
  uint32_t arr[4]    = {0x1, 0x2, 0x3, 0x4};
  uint32_t original[4];
  std::memcpy(original, arr, sizeof(arr));

  g_permute_count = 0;
  utils_permute(arr, RCSW_ARRAY_ELTS(arr), sizeof(uint32_t), 0,
                permute_counter_cb);

  CATCH_REQUIRE(std::memcmp(arr, original, sizeof(arr)) == 0);
}

static void permute_partial_test() {
  /*
   * Passing start > 0 should only permute the suffix [start, n_elts).
   * For arr = {A, B, C, D} with start = 2, only {C, D} are permuted:
   * 2! = 2 callbacks.
   */
  uint32_t arr[4] = {0x1, 0x2, 0x3, 0x4};
  g_permute_count = 0;
  utils_permute(arr, RCSW_ARRAY_ELTS(arr), sizeof(uint32_t), 2,
                permute_counter_cb);
  CATCH_REQUIRE(g_permute_count == 2u);

  /* The prefix must be untouched */
  CATCH_REQUIRE(arr[0] == 0x1u);
  CATCH_REQUIRE(arr[1] == 0x2u);
}

static void zchk_test() {
  /* All-zero elements of each native size */
  uint8_t  z8  = 0;
  uint16_t z16 = 0;
  uint32_t z32 = 0;
  double   zd  = 0.0;
  CATCH_REQUIRE(utils_zchk(&z8,  sizeof(z8))  == true);
  CATCH_REQUIRE(utils_zchk(&z16, sizeof(z16)) == true);
  CATCH_REQUIRE(utils_zchk(&z32, sizeof(z32)) == true);
  CATCH_REQUIRE(utils_zchk(&zd,  sizeof(zd))  == true);

  /* Non-zero elements must return false */
  uint8_t  nz8  = 1;
  uint16_t nz16 = 1;
  uint32_t nz32 = 1;
  double   nzd  = 1.0;
  CATCH_REQUIRE(utils_zchk(&nz8,  sizeof(nz8))  == false);
  CATCH_REQUIRE(utils_zchk(&nz16, sizeof(nz16)) == false);
  CATCH_REQUIRE(utils_zchk(&nz32, sizeof(nz32)) == false);
  CATCH_REQUIRE(utils_zchk(&nzd,  sizeof(nzd))  == false);

  /* Large all-zero struct falls through to the byte loop */
  uint8_t big[64] = {};
  CATCH_REQUIRE(utils_zchk(big, sizeof(big)) == true);
  big[32] = 1;
  CATCH_REQUIRE(utils_zchk(big, sizeof(big)) == false);

  /* NULL pointer must return false (FPC guard) */
  CATCH_REQUIRE(utils_zchk(nullptr, sizeof(uint32_t)) == false);
}

static void clamp_f255_test() {
  /* Values inside [0, 255] pass through unchanged */
  CATCH_REQUIRE(utils_clamp_f255(0.0f)   == 0.0f);
  CATCH_REQUIRE(utils_clamp_f255(127.5f) == 127.5f);
  CATCH_REQUIRE(utils_clamp_f255(255.0f) == 255.0f);

  /* Negative values clamp to 0 */
  CATCH_REQUIRE(utils_clamp_f255(-1.0f)    == 0.0f);
  CATCH_REQUIRE(utils_clamp_f255(-1000.0f) == 0.0f);

  /* Values above 255 clamp to 255 */
  CATCH_REQUIRE(utils_clamp_f255(255.001f) == 255.0f);
  CATCH_REQUIRE(utils_clamp_f255(1000.0f)  == 255.0f);
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("Numeric Permute Count Test", "[utils][numeric]") {
  permute_count_test();
}
CATCH_TEST_CASE("Numeric Permute Restores Test", "[utils][numeric]") {
  permute_restores_test();
}
CATCH_TEST_CASE("Numeric Permute Partial Test", "[utils][numeric]") {
  permute_partial_test();
}
CATCH_TEST_CASE("Numeric zchk Test", "[utils][numeric]") {
  zchk_test();
}
CATCH_TEST_CASE("Numeric clamp_f255 Test", "[utils][numeric]") {
  clamp_f255_test();
}
