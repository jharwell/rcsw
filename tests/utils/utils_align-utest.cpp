/**
 * \file utils_align-utest.cpp
 *
 * Unit tests for rcsw/utils/align.h and rcsw/utils/endian.h.
 *
 * Covers:
 *   align.h  — RCSW_IS_MEM_ALIGNED, RCSW_IS_SIZE_ALIGNED, RCSW_ALIGN_SIZE
 *   endian.h — utils_is_little_endian(), utils_is_big_endian()
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

#include <cstdint>
#include <cstddef>

#include "rcsw/utils/align.h"
#include "rcsw/utils/endian.h"

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/

static void is_mem_aligned_test() {
  /* A 16-byte aligned buffer is aligned to 1, 2, 4, 8, and 16 bytes */
  alignas(16) uint8_t buf[64];
  CATCH_REQUIRE(RCSW_IS_MEM_ALIGNED(buf, 1));
  CATCH_REQUIRE(RCSW_IS_MEM_ALIGNED(buf, 2));
  CATCH_REQUIRE(RCSW_IS_MEM_ALIGNED(buf, 4));
  CATCH_REQUIRE(RCSW_IS_MEM_ALIGNED(buf, 8));
  CATCH_REQUIRE(RCSW_IS_MEM_ALIGNED(buf, 16));

  /* Deliberately misaligned pointer (buf + 1 is never 2-byte aligned) */
  uint8_t* misaligned = buf + 1;
  CATCH_REQUIRE(!RCSW_IS_MEM_ALIGNED(misaligned, 2));
  CATCH_REQUIRE(!RCSW_IS_MEM_ALIGNED(misaligned, 4));

  /* uintptr_t arithmetic: offset by 3 from a 4-aligned address is not 4-aligned */
  uint8_t* off3 = buf + 3;
  CATCH_REQUIRE(!RCSW_IS_MEM_ALIGNED(off3, 4));

  /* NULL is trivially 0 mod anything → aligned to any power of two */
  CATCH_REQUIRE(RCSW_IS_MEM_ALIGNED(nullptr, 4));
}

static void is_size_aligned_test() {
  /* Exact multiples are aligned */
  CATCH_REQUIRE(RCSW_IS_SIZE_ALIGNED(0,   4));
  CATCH_REQUIRE(RCSW_IS_SIZE_ALIGNED(4,   4));
  CATCH_REQUIRE(RCSW_IS_SIZE_ALIGNED(8,   4));
  CATCH_REQUIRE(RCSW_IS_SIZE_ALIGNED(16,  4));
  CATCH_REQUIRE(RCSW_IS_SIZE_ALIGNED(64,  8));
  CATCH_REQUIRE(RCSW_IS_SIZE_ALIGNED(128, 16));

  /* Non-multiples are not aligned */
  CATCH_REQUIRE(!RCSW_IS_SIZE_ALIGNED(1,  4));
  CATCH_REQUIRE(!RCSW_IS_SIZE_ALIGNED(3,  4));
  CATCH_REQUIRE(!RCSW_IS_SIZE_ALIGNED(5,  4));
  CATCH_REQUIRE(!RCSW_IS_SIZE_ALIGNED(7,  8));
  CATCH_REQUIRE(!RCSW_IS_SIZE_ALIGNED(15, 16));
}

static void align_size_test() {
  /* Values already aligned are unchanged */
  CATCH_REQUIRE(RCSW_ALIGN_SIZE(0,  4) == 0u);
  CATCH_REQUIRE(RCSW_ALIGN_SIZE(4,  4) == 4u);
  CATCH_REQUIRE(RCSW_ALIGN_SIZE(8,  4) == 8u);
  CATCH_REQUIRE(RCSW_ALIGN_SIZE(16, 8) == 16u);

  /* Unaligned values round up to the next multiple */
  CATCH_REQUIRE(RCSW_ALIGN_SIZE(1,  4) == 4u);
  CATCH_REQUIRE(RCSW_ALIGN_SIZE(3,  4) == 4u);
  CATCH_REQUIRE(RCSW_ALIGN_SIZE(5,  4) == 8u);
  CATCH_REQUIRE(RCSW_ALIGN_SIZE(7,  8) == 8u);
  CATCH_REQUIRE(RCSW_ALIGN_SIZE(9,  8) == 16u);
  CATCH_REQUIRE(RCSW_ALIGN_SIZE(15, 16) == 16u);
  CATCH_REQUIRE(RCSW_ALIGN_SIZE(17, 16) == 32u);

  /* Result must always be IS_SIZE_ALIGNED */
  for (size_t s = 0; s <= 64; ++s) {
    CATCH_REQUIRE(RCSW_IS_SIZE_ALIGNED(RCSW_ALIGN_SIZE(s, 4), 4));
    CATCH_REQUIRE(RCSW_IS_SIZE_ALIGNED(RCSW_ALIGN_SIZE(s, 8), 8));
  }
}

static void endian_test() {
  /* Exactly one of little-endian or big-endian must be true */
  bool le = utils_is_little_endian();
  bool be = utils_is_big_endian();
  CATCH_REQUIRE(le != be);

  /* The result must be consistent with the byte layout of uint16_t(1) */
  union {
    uint16_t u;
    uint8_t  c[2];
  } probe = {.u = 1};

  if (probe.c[0] == 1) {
    CATCH_REQUIRE(le == true);
    CATCH_REQUIRE(be == false);
  } else {
    CATCH_REQUIRE(le == false);
    CATCH_REQUIRE(be == true);
  }
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("Align IS_MEM_ALIGNED Test", "[utils][align]") {
  is_mem_aligned_test();
}
CATCH_TEST_CASE("Align IS_SIZE_ALIGNED Test", "[utils][align]") {
  is_size_aligned_test();
}
CATCH_TEST_CASE("Align ALIGN_SIZE Test", "[utils][align]") {
  align_size_test();
}
CATCH_TEST_CASE("Endian Detection Test", "[utils][endian]") {
  endian_test();
}
