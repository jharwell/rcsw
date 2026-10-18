/**
 * \file
 *
 * \copyright 2026 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * Unit tests for rcsw/tool/grind.h built with RCSW_CONFIG_NO_GRIND: the
 * RCSW_GRIND_* macros must compile out entirely.
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#define RCSW_CONFIG_NO_GRIND
#define CATCH_CONFIG_PREFIX_ALL
#include <catch2/catch_test_macros.hpp>

#include "rcsw/tool/grind.h"

/*******************************************************************************
 * Test Helpers
 ******************************************************************************/
namespace {
int g_evaluated = 0;

/* Only named inside the compiled-out macros, so never actually called */
[[maybe_unused]] struct grinder* touch() {
  ++g_evaluated;
  return nullptr;
}
} // namespace

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("RCSW_CONFIG_NO_GRIND compiles the RCSW_GRIND_* macros out",
                "[tool][grind]") {
  g_evaluated = 0;
  /* Arguments are never evaluated, so a NULL grinder is never used */
  RCSW_GRIND_START(touch(), 0);
  RCSW_GRIND_END(touch(), 0);
  RCSW_GRIND_COUNT(touch(), 0);
  RCSW_GRIND_TICK(touch(), 0);
  CATCH_REQUIRE(0 == g_evaluated);

  /* Each expands to one statement, so it is safe in an unbraced if/else */
  bool flag = (g_evaluated == 0);
  if (flag)
    RCSW_GRIND_TICK(touch(), 0);
  else
    RCSW_GRIND_COUNT(touch(), 0);
  CATCH_REQUIRE(0 == g_evaluated);
}
