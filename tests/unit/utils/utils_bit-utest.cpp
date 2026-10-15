/**
 * \file
 *
 * \copyright 2023 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * Unit tests for rcsw/utils/bit.h: RCSW_TOPBIT().
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
CATCH_TEST_CASE("Bit TOPBIT Test", "[utils][bit][noalloc]") { topbit_test(); }
