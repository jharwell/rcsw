/**
 * \file utils_byteops-utest.cpp
 *
 * Unit tests for rcsw/utils/byteops.h: byte-swap macros (BSWAP16/32/64,
 * WSWAP32), utils_arr8_reverse(), utils_elt_swap(), and utils_string_gen().
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

#include "rcsw/utils/byteops.h"

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/

static void bswap_macros_test() {
  /* Single active byte migrates to the opposite end */
  CATCH_REQUIRE(RCSW_BSWAP16(uint16_t(0x0001)) == 0x0100u);
  CATCH_REQUIRE(RCSW_BSWAP32(uint32_t(0x00000001)) == 0x01000000u);
  CATCH_REQUIRE(RCSW_BSWAP64(uint64_t(0x0000000000000001)) == UINT64_C(0x0100000000000000));

  CATCH_REQUIRE(RCSW_BSWAP16(uint16_t(0x8000)) == 0x0080u);
  CATCH_REQUIRE(RCSW_BSWAP32(uint32_t(0x80000000)) == 0x00000080u);
  CATCH_REQUIRE(RCSW_BSWAP64(uint64_t(0x8000000000000000)) == UINT64_C(0x0000000000000080));

  /* Known byte-reversal patterns */
  CATCH_REQUIRE(RCSW_BSWAP16(uint16_t(0x0123)) == 0x2301u);
  CATCH_REQUIRE(RCSW_BSWAP32(uint32_t(0x01234567)) == 0x67452301u);
  CATCH_REQUIRE(RCSW_BSWAP64(uint64_t(0x0123456789ABCDEFull)) == UINT64_C(0xEFCDAB8967452301));

  /* Applying BSWAP twice restores the original value */
  uint16_t v16 = 0xABCDu;
  uint32_t v32 = 0x01234567u;
  uint64_t v64 = UINT64_C(0x0123456789ABCDEF);
  CATCH_REQUIRE(RCSW_BSWAP16(RCSW_BSWAP16(v16)) == v16);
  CATCH_REQUIRE(RCSW_BSWAP32(RCSW_BSWAP32(v32)) == v32);
  CATCH_REQUIRE(RCSW_BSWAP64(RCSW_BSWAP64(v64)) == v64);
}

static void wswap32_test() {
  /* WSWAP32 swaps the two 16-bit halves of a 32-bit word */
  CATCH_REQUIRE(RCSW_WSWAP32(uint32_t(0x00010000)) == 0x00000001u);
  CATCH_REQUIRE(RCSW_WSWAP32(uint32_t(0x12340000)) == 0x00001234u);
  CATCH_REQUIRE(RCSW_WSWAP32(uint32_t(0xABCD1234)) == 0x1234ABCDu);

  /* Applying WSWAP twice restores the original */
  uint32_t v = 0xDEADBEEFu;
  CATCH_REQUIRE(RCSW_WSWAP32(RCSW_WSWAP32(v)) == v);
}

static void arr8_reverse_test() {
  /* Odd-length array: centre element stays, rest swap pairwise */
  uint8_t arr[5]    = {0x5, 0x1, 0x2, 0x3, 0x4};
  uint8_t expected[5] = {0x4, 0x3, 0x2, 0x1, 0x5};
  utils_arr8_reverse(arr, RCSW_ARRAY_ELTS(arr));
  CATCH_REQUIRE(std::memcmp(arr, expected, sizeof(arr)) == 0);

  /* Even-length array */
  uint8_t even[4]    = {0x1, 0x2, 0x3, 0x4};
  uint8_t even_exp[4] = {0x4, 0x3, 0x2, 0x1};
  utils_arr8_reverse(even, RCSW_ARRAY_ELTS(even));
  CATCH_REQUIRE(std::memcmp(even, even_exp, sizeof(even)) == 0);

  /* Reversing twice restores the original */
  uint8_t orig[5] = {0x1, 0x2, 0x3, 0x4, 0x5};
  uint8_t copy[5];
  std::memcpy(copy, orig, sizeof(orig));
  utils_arr8_reverse(copy, RCSW_ARRAY_ELTS(copy));
  utils_arr8_reverse(copy, RCSW_ARRAY_ELTS(copy));
  CATCH_REQUIRE(std::memcmp(copy, orig, sizeof(orig)) == 0);

  /* Single-element: must not crash or corrupt */
  uint8_t one[1] = {0x42};
  utils_arr8_reverse(one, 1);
  CATCH_REQUIRE(one[0] == 0x42u);
}

static void elt_swap_test() {
  /* Swap adjacent elements in a uint32_t array */
  uint32_t arr[4]    = {0x4, 0x1, 0x2, 0x3};
  uint32_t expected[4] = {0x1, 0x4, 0x3, 0x2};
  utils_elt_swap(arr, sizeof(uint32_t), 0, 1);
  utils_elt_swap(arr, sizeof(uint32_t), 2, 3);
  CATCH_REQUIRE(std::memcmp(arr, expected, sizeof(arr)) == 0);

  /* Swapping an element with itself must be a no-op */
  uint32_t same[2] = {0xDEAD, 0xBEEF};
  utils_elt_swap(same, sizeof(uint32_t), 0, 0);
  CATCH_REQUIRE(same[0] == 0xDEADu);
  CATCH_REQUIRE(same[1] == 0xBEEFu);

  /* Swap on a byte array */
  uint8_t bytes[4]    = {0xAA, 0xBB, 0xCC, 0xDD};
  uint8_t bytes_exp[4] = {0xDD, 0xBB, 0xCC, 0xAA};
  utils_elt_swap(bytes, sizeof(uint8_t), 0, 3);
  CATCH_REQUIRE(std::memcmp(bytes, bytes_exp, sizeof(bytes)) == 0);
}

static void string_gen_test() {
  /* utils_string_gen fills len-1 printable ASCII characters + NUL */
  char buf[64];
  std::memset(buf, 0, sizeof(buf));
  CATCH_REQUIRE(utils_string_gen(buf, sizeof(buf)) == OK);

  /* Every character before the NUL must be printable ASCII [33, 126] */
  for (size_t i = 0; i < sizeof(buf) - 1; ++i) {
    CATCH_REQUIRE((unsigned char)buf[i] >= 33u);
    CATCH_REQUIRE((unsigned char)buf[i] <= 126u);
  }
  CATCH_REQUIRE(buf[sizeof(buf) - 1] == '\0');

  /* Minimum viable length: 1-character string (len=2, 1 char + NUL) */
  char small[2] = {'\xFF', '\xFF'};
  CATCH_REQUIRE(utils_string_gen(small, sizeof(small)) == OK);
  CATCH_REQUIRE(small[1] == '\0');

  /* NULL buffer must not crash (returns ERROR) */
  CATCH_REQUIRE(utils_string_gen(nullptr, 10) == ERROR);
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("Byteops BSWAP Macros Test", "[utils][byteops]") {
  bswap_macros_test();
}
CATCH_TEST_CASE("Byteops WSWAP32 Test", "[utils][byteops]") {
  wswap32_test();
}
CATCH_TEST_CASE("Byteops arr8_reverse Test", "[utils][byteops]") {
  arr8_reverse_test();
}
CATCH_TEST_CASE("Byteops elt_swap Test", "[utils][byteops]") {
  elt_swap_test();
}
CATCH_TEST_CASE("Byteops string_gen Test", "[utils][byteops]") {
  string_gen_test();
}
