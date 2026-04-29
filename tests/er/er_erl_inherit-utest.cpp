/**
 * \file er_erl_inherit-utest.cpp
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

/*
 * Simulate a build system that globally defines LIBRA_ERL, then a per-file
 * header that opts in to inheriting the outer level rather than hardcoding one.
 */
#define LIBRA_ERL         2 /* RCSW_ERL_ERROR */
#define LIBRA_ERL_INHERIT

#define RCSW_ER_MODNAME "test.er.inherit"
#define RCSW_ER_MODID   (0x0020)

#define RCSW_CONFIG_ER_PLUGIN 1

#include "rcsw/er/er.h"
#include "rcsw/er/plugin/simple.h"
#include "rcsw/er/macros.h"

#define CATCH_CONFIG_PREFIX_ALL
#include <catch2/catch_test_macros.hpp>

#include "tests/test_utils.hpp"

/*******************************************************************************
 * Compile-time Invariants
 ******************************************************************************/
static_assert(RCSW_ERL == 2 /* RCSW_ERL_ERROR */,
              "LIBRA_ERL_INHERIT must forward LIBRA_ERL=2 to RCSW_ERL");
static_assert(RCSW_ERL >= RCSW_ERL_FATAL, "inherited ERROR includes FATAL");
static_assert(RCSW_ERL >= RCSW_ERL_ERROR, "inherited ERROR includes ERROR");
static_assert(RCSW_ERL <  RCSW_ERL_WARN,  "inherited ERROR excludes WARN+");

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/

static void inherit_level_test() {
  /* With the inherited ERL_ERROR level, FATAL and ERROR must emit. */
  CATCH_REQUIRE(capture_stdout([] { ER_FATAL("fatal at inherited"); }).find("[FATAL]") != std::string::npos);
  CATCH_REQUIRE(capture_stdout([] { ER_ERR  ("error at inherited"); }).find("[ERROR]") != std::string::npos);

  /* WARN and above are suppressed. */
  CATCH_REQUIRE(capture_stdout([] { ER_WARN ("warn suppressed");  }).empty());
  CATCH_REQUIRE(capture_stdout([] { ER_INFO ("info suppressed");  }).empty());
  CATCH_REQUIRE(capture_stdout([] { ER_DEBUG("debug suppressed"); }).empty());
  CATCH_REQUIRE(capture_stdout([] { ER_TRACE("trace suppressed"); }).empty());
}

static void inherit_dprintf_test() {
  /* DPRINTF is available at any level > NONE; must emit at inherited ERL_ERROR. */
  CATCH_REQUIRE(capture_stdout([] { DPRINTF("visible\n"); }) == "visible\n");
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("ER ERL Inherit Level Test", "[er][erl_inherit]") {
  inherit_level_test();
}
CATCH_TEST_CASE("ER ERL Inherit DPRINTF Test", "[er][erl_inherit]") {
  inherit_dprintf_test();
}
