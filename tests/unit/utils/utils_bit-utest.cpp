/**
 * \file
 *
 * \copyright 2023 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * Unit tests for rcsw/utils/bit.h: RCSW_BIT_WIDTH().
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#define CATCH_CONFIG_PREFIX_ALL
#include <catch2/catch_test_macros.hpp>

#include <cstdint>

#include "rcsw/utils/bit.h"

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/

static void bit_width_test() {
  CATCH_REQUIRE(RCSW_BIT_WIDTH(uint8_t) == 8);
  CATCH_REQUIRE(RCSW_BIT_WIDTH(uint16_t) == 16);
  CATCH_REQUIRE(RCSW_BIT_WIDTH(uint32_t) == 32);
  CATCH_REQUIRE(RCSW_BIT_WIDTH(uint64_t) == 64);
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("Bit width Test", "[utils][bit][noalloc]") { bit_width_test(); }
