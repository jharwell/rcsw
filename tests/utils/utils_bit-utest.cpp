/**
 * \file utils_bit-utest.cpp
 *
 * Unit tests for rcsw/utils/bit.h: bit reversal (shift and table variants),
 * bit reflection, half-word masking, binary literal macros, and
 * utils_reflect32().
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

#include "rcsw/utils/bit.h"

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/

static void rev_shift_test() {
  /* Single-bit values: MSB ↔ LSB */
  CATCH_REQUIRE(RCSW_REV8(0x01u)  == 0x80u);
  CATCH_REQUIRE(RCSW_REV16(0x0001u) == 0x8000u);
  CATCH_REQUIRE(RCSW_REV32(0x00000001u) == 0x80000000u);

  /* Inverse direction */
  CATCH_REQUIRE(RCSW_REV8(0x80u)  == 0x01u);
  CATCH_REQUIRE(RCSW_REV16(0x8000u) == 0x0001u);
  CATCH_REQUIRE(RCSW_REV32(0x80000000u) == 0x00000001u);

  /* Multi-bit patterns */
  CATCH_REQUIRE(RCSW_REV8(0xF0u) == 0x0Fu);
  CATCH_REQUIRE(RCSW_REV8(0xAAu) == 0x55u); /* 10101010 → 01010101 */
  CATCH_REQUIRE(RCSW_REV16(0xFF00u) == 0x00FFu);
  CATCH_REQUIRE(RCSW_REV32(0xFFFF0000u) == 0x0000FFFFu);
}

static void rev_table_test() {
  /* Table variant must produce identical results to the shift variant. */
  CATCH_REQUIRE(RCSW_REVTBL8(0x01u)  == 0x80u);
  CATCH_REQUIRE(RCSW_REVTBL16(0x0001u) == 0x8000u);
  CATCH_REQUIRE(RCSW_REVTBL32(0x00000001u) == 0x80000000u);

  CATCH_REQUIRE(RCSW_REVTBL8(0x80u)  == 0x01u);
  CATCH_REQUIRE(RCSW_REVTBL16(0x8000u) == 0x0001u);
  CATCH_REQUIRE(RCSW_REVTBL32(0x80000000u) == 0x00000001u);

  /* Spot-check agreement with the shift variant for additional patterns. */
  for (uint8_t v : {uint8_t(0x00), uint8_t(0xAA), uint8_t(0x55),
                    uint8_t(0x0F), uint8_t(0xF0), uint8_t(0xFF)}) {
    CATCH_REQUIRE(RCSW_REVTBL8(v) == RCSW_REV8(v));
  }
}

static void bitmask_test() {
  /* RCSW_BITS_HI32 / RCSW_BITS_LO32 split a 32-bit value at bit 16 */
  uint32_t foo32 = 0x34567890u;
  CATCH_REQUIRE(RCSW_BITS_HI32(foo32) == 0x34560000u);
  CATCH_REQUIRE(RCSW_BITS_LO32(foo32) == 0x00007890u);

  /* Together they reconstitute the original value */
  CATCH_REQUIRE((RCSW_BITS_HI32(foo32) | RCSW_BITS_LO32(foo32)) == foo32);

  /* RCSW_BITS_HI64 / RCSW_BITS_LO64 split a 64-bit value at bit 32 */
  uint64_t foo64 = UINT64_C(0xFFAABBEE00112233);
  CATCH_REQUIRE(RCSW_BITS_HI64(foo64) == UINT64_C(0xFFAABBEE00000000));
  CATCH_REQUIRE(RCSW_BITS_LO64(foo64) == UINT64_C(0x0000000000112233));

  CATCH_REQUIRE((RCSW_BITS_HI64(foo64) | RCSW_BITS_LO64(foo64)) == foo64);
}

static void bin_literal_test() {
  /* RCSW_BIN8: pseudo-binary → true uint8 value */
  CATCH_REQUIRE(RCSW_BIN8(00000000) == 0x00u);
  CATCH_REQUIRE(RCSW_BIN8(11111111) == 0xFFu);
  CATCH_REQUIRE(RCSW_BIN8(10000000) == 0x80u);
  CATCH_REQUIRE(RCSW_BIN8(00000001) == 0x01u);
  CATCH_REQUIRE(RCSW_BIN8(10101010) == 0xAAu);
  CATCH_REQUIRE(RCSW_BIN8(01010101) == 0x55u);

  /* RCSW_BIN16 / RCSW_BIN32 compose from byte pieces */
  CATCH_REQUIRE(RCSW_BIN16(00000001, 00000000) == 0x0100u);
  CATCH_REQUIRE(RCSW_BIN32(00000001, 00000000, 00000000, 00000000) == 0x01000000u);
}

static void topbit_test() {
  /* RCSW_TOPBIT isolates the MSB of each type */
  CATCH_REQUIRE(RCSW_TOPBIT(uint8_t)  == 0x80u);
  CATCH_REQUIRE(RCSW_TOPBIT(uint16_t) == 0x8000u);
  CATCH_REQUIRE(RCSW_TOPBIT(uint32_t) == 0x80000000u);
}

static void reflect32_test() {
  /* utils_reflect32(v, 32): full 32-bit reflection */
  CATCH_REQUIRE(utils_reflect32(0xF0021001u, 32) == 0x8008400Fu);

  /* utils_reflect32(v, 16): reflect lower 16 bits, upper half zeroed */
  CATCH_REQUIRE(utils_reflect32(0xF0000001u, 16) == 0x8000u);

  /* Identity check: reflecting twice restores original for 32-bit */
  uint32_t orig = 0x12345678u;
  CATCH_REQUIRE(utils_reflect32(utils_reflect32(orig, 32), 32) == orig);

  /* Single bit: bit 0 → bit n_bits-1 */
  CATCH_REQUIRE(utils_reflect32(1u, 8)  == 0x80u);
  CATCH_REQUIRE(utils_reflect32(1u, 16) == 0x8000u);
  CATCH_REQUIRE(utils_reflect32(1u, 32) == 0x80000000u);
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("Bit Reverse Shift Test", "[utils][bit]") {
  rev_shift_test();
}
CATCH_TEST_CASE("Bit Reverse Table Test", "[utils][bit]") {
  rev_table_test();
}
CATCH_TEST_CASE("Bit Bitmask Test", "[utils][bit]") {
  bitmask_test();
}
CATCH_TEST_CASE("Bit Binary Literal Test", "[utils][bit]") {
  bin_literal_test();
}
CATCH_TEST_CASE("Bit TOPBIT Test", "[utils][bit]") {
  topbit_test();
}
CATCH_TEST_CASE("Bit Reflect32 Test", "[utils][bit]") {
  reflect32_test();
}
