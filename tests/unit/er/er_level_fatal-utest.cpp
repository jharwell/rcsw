/**
 * \file
 *
 * \copyright 2026 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * ERL = FATAL with no RCSW_ER_MODNAME defined: ER_FATAL uses the default
 * module name (__FILE_NAME__), which must be defined at every ER level.
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#define LIBRA_ERL 1 /* RCSW_ERL_FATAL */

#define RCSW_CONFIG_ER_PLUGIN 1
#include "rcsw/er/er.h"
#include "rcsw/er/macros.h"
#include "rcsw/er/plugin/simple.h"

#define CATCH_CONFIG_PREFIX_ALL
#include <catch2/catch_test_macros.hpp>

#include "tests/unit/test_utils.hpp"

static_assert(RCSW_ERL == RCSW_ERL_FATAL, "ERL must be FATAL in this TU");

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("ER_FATAL at ERL_FATAL uses the default module name",
                "[er][noalloc]") {
  std::string out = capture_stdout([] { ER_FATAL("boom"); });
  CATCH_REQUIRE(out.find("boom") != std::string::npos);
  CATCH_REQUIRE(out.find("er_level_fatal-utest") != std::string::npos);
}
