/**
 * \file
 *
 * \copyright 2023 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * Unit tests for rcsw/utils/bit.h: half-word masking, binary literal macros,
 * and RCSW_TOPBIT().
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
  CATCH_REQUIRE(RCSW_BIN32(00000001, 00000000, 00000000, 00000000) ==
                0x01000000u);
  CATCH_REQUIRE(RCSW_BIN16(11110000, 00001111) == 0xF00Fu);

  /* the top byte reaches bit 31 without signed overflow */
  CATCH_REQUIRE(RCSW_BIN32(10000000, 00000000, 00000000, 00000001) ==
                0x80000001u);
  CATCH_REQUIRE(RCSW_BIN32(11111111, 11111111, 11111111, 11111111) ==
                0xFFFFFFFFu);
}

static void topbit_test() {
  /* RCSW_TOPBIT isolates the MSB of each type */
  CATCH_REQUIRE(RCSW_TOPBIT(uint8_t) == 0x80u);
  CATCH_REQUIRE(RCSW_TOPBIT(uint16_t) == 0x8000u);
  CATCH_REQUIRE(RCSW_TOPBIT(uint32_t) == 0x80000000u);
  CATCH_REQUIRE(RCSW_TOPBIT(uint64_t) == 0x8000000000000000ull);
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("Bit Bitmask Test", "[utils][bit][noalloc]") { bitmask_test(); }
CATCH_TEST_CASE("Bit Binary Literal Test", "[utils][bit][noalloc]") {
  bin_literal_test();
}
CATCH_TEST_CASE("Bit TOPBIT Test", "[utils][bit][noalloc]") { topbit_test(); }
