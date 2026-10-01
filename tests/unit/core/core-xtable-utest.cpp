/**
 * \file
 *
 * \copyright 2026 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * RCSW_XTABLE_STR() generates a table of stringified entries.
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#define CATCH_CONFIG_PREFIX_ALL
#include <cstring>

#include <catch2/catch_test_macros.hpp>

#include "rcsw/core/core.h"

#define TEST_ENTRIES ALPHA, BETA, GAMMA

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("RCSW_XTABLE_STR generates a string table",
                "[core][xtable][noalloc]") {
  const char* names[] = {RCSW_XTABLE_STR(TEST_ENTRIES)};
  CATCH_REQUIRE(3 == RCSW_ARRAY_ELTS(names));
  CATCH_REQUIRE(0 == strcmp(names[0], "ALPHA"));
  CATCH_REQUIRE(0 == strcmp(names[2], "GAMMA"));
}
