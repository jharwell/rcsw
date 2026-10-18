/**
 * \file
 *
 * \copyright 2023 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * Unit tests for rcsw/utils/align.h.
 *
 * Covers:
 *   align.h  — utils_is_pow2(), utils_is_addr_aligned(), utils_is_mem_aligned(),
 *              utils_is_size_aligned(), utils_align_size()
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#define CATCH_CONFIG_PREFIX_ALL
#include <cstddef>
#include <cstdint>

#include <catch2/catch_test_macros.hpp>

#include "rcsw/utils/align.h"

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/

static void is_mem_aligned_test() {
  /* A 16-byte aligned buffer is aligned to 1, 2, 4, 8, and 16 bytes */
  alignas(16) uint8_t buf[64];
  CATCH_REQUIRE(utils_is_mem_aligned(buf, 1));
  CATCH_REQUIRE(utils_is_mem_aligned(buf, 2));
  CATCH_REQUIRE(utils_is_mem_aligned(buf, 4));
  CATCH_REQUIRE(utils_is_mem_aligned(buf, 8));
  CATCH_REQUIRE(utils_is_mem_aligned(buf, 16));

  /* Deliberately misaligned pointer (buf + 1 is never 2-byte aligned) */
  uint8_t* misaligned = buf + 1;
  CATCH_REQUIRE(!utils_is_mem_aligned(misaligned, 2));
  CATCH_REQUIRE(!utils_is_mem_aligned(misaligned, 4));

  /* uintptr_t arithmetic: offset by 3 from a 4-aligned address is not 4-aligned
   */
  uint8_t* off3 = buf + 3;
  CATCH_REQUIRE(!utils_is_mem_aligned(off3, 4));

  /* NULL is trivially 0 mod anything, so aligned to everything */
  CATCH_REQUIRE(utils_is_mem_aligned(nullptr, 4));
}

static void is_size_aligned_test() {
  /* Exact multiples are aligned */
  CATCH_REQUIRE(utils_is_size_aligned(0, 4));
  CATCH_REQUIRE(utils_is_size_aligned(4, 4));
  CATCH_REQUIRE(utils_is_size_aligned(8, 4));
  CATCH_REQUIRE(utils_is_size_aligned(16, 4));
  CATCH_REQUIRE(utils_is_size_aligned(64, 8));
  CATCH_REQUIRE(utils_is_size_aligned(128, 16));

  /* Non-multiples are not aligned */
  CATCH_REQUIRE(!utils_is_size_aligned(1, 4));
  CATCH_REQUIRE(!utils_is_size_aligned(3, 4));
  CATCH_REQUIRE(!utils_is_size_aligned(5, 4));
  CATCH_REQUIRE(!utils_is_size_aligned(7, 8));
  CATCH_REQUIRE(!utils_is_size_aligned(15, 16));
}

static void align_size_test() {
  /* Values already aligned are unchanged */
  CATCH_REQUIRE(utils_align_size(0, 4) == 0u);
  CATCH_REQUIRE(utils_align_size(4, 4) == 4u);
  CATCH_REQUIRE(utils_align_size(8, 4) == 8u);
  CATCH_REQUIRE(utils_align_size(16, 8) == 16u);

  /* Unaligned values round up to the next multiple */
  CATCH_REQUIRE(utils_align_size(1, 4) == 4u);
  CATCH_REQUIRE(utils_align_size(3, 4) == 4u);
  CATCH_REQUIRE(utils_align_size(5, 4) == 8u);
  CATCH_REQUIRE(utils_align_size(7, 8) == 8u);
  CATCH_REQUIRE(utils_align_size(9, 8) == 16u);
  CATCH_REQUIRE(utils_align_size(15, 16) == 16u);
  CATCH_REQUIRE(utils_align_size(17, 16) == 32u);

  /* Result must always be size-aligned */
  for (size_t s = 0; s <= 64; ++s) {
    CATCH_REQUIRE(utils_is_size_aligned(utils_align_size(s, 4), 4));
    CATCH_REQUIRE(utils_is_size_aligned(utils_align_size(s, 8), 8));
  }
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("utils_is_mem_aligned Test", "[utils][align][noalloc]") {
  is_mem_aligned_test();
}
CATCH_TEST_CASE("utils_is_size_aligned Test", "[utils][align][noalloc]") {
  is_size_aligned_test();
}
CATCH_TEST_CASE("utils_align_size Test", "[utils][align][noalloc]") {
  align_size_test();
}

CATCH_TEST_CASE("utils_align_size works in the full width of size_t",
                "[utils][align][noalloc]") {
#if SIZE_MAX > UINT32_MAX
  size_t big = static_cast<size_t>(0x100000005ULL);
  CATCH_REQUIRE(static_cast<size_t>(0x100000008ULL) == utils_align_size(big, 8));
#endif
  CATCH_REQUIRE(8U == utils_align_size(5U, 8));
  CATCH_REQUIRE(16U == utils_align_size(13, 4));
}

CATCH_TEST_CASE("utils_is_pow2", "[utils][align][noalloc]") {
  CATCH_REQUIRE(!utils_is_pow2(0));
  CATCH_REQUIRE(utils_is_pow2(1));
  CATCH_REQUIRE(utils_is_pow2(2));
  CATCH_REQUIRE(!utils_is_pow2(3));
  CATCH_REQUIRE(utils_is_pow2(4096));
  CATCH_REQUIRE(!utils_is_pow2(4097));
  CATCH_REQUIRE(utils_is_pow2(SIZE_MAX / 2 + 1));
}

CATCH_TEST_CASE("Alignment preconditions fail cleanly",
                "[utils][align][noalloc]") {
  /* Under LIBRA_FPC_RETURN: non-power-of-two and zero alignments */
  CATCH_REQUIRE(!utils_is_size_aligned(6, 3));
  CATCH_REQUIRE(!utils_is_size_aligned(0, 0));
  CATCH_REQUIRE(0U == utils_align_size(5, 3));
  CATCH_REQUIRE(0U == utils_align_size(5, 0));
  int x = 0;
  CATCH_REQUIRE(!utils_is_mem_aligned(&x, 0));
  CATCH_REQUIRE(!utils_is_addr_aligned(16, 0));
  /* Address/pointer alignment accepts any nonzero alignment */
  CATCH_REQUIRE(utils_is_addr_aligned(12, 3));
  CATCH_REQUIRE(!utils_is_addr_aligned(13, 3));
  CATCH_REQUIRE(utils_is_addr_aligned(0, 7));
}
