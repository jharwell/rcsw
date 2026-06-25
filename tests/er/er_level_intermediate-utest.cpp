/**
 * \file er_level_intermediate-utest.cpp
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#define LIBRA_ERL 3 /* RCSW_ERL_WARN */

#define RCSW_ER_MODNAME "test.er.warn"
#define RCSW_ER_MODID   (0x0003)

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
static_assert(RCSW_ERL == RCSW_ERL_WARN,  "ERL must be WARN in this TU");
static_assert(RCSW_ERL >= RCSW_ERL_FATAL, "WARN includes FATAL");
static_assert(RCSW_ERL >= RCSW_ERL_ERROR, "WARN includes ERROR");
static_assert(RCSW_ERL >= RCSW_ERL_WARN,  "WARN includes WARN");
static_assert(RCSW_ERL <  RCSW_ERL_INFO,  "WARN excludes INFO");
static_assert(RCSW_ERL <  RCSW_ERL_DEBUG, "WARN excludes DEBUG");
static_assert(RCSW_ERL <  RCSW_ERL_TRACE, "WARN excludes TRACE");

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/

static void emission_test() {
  /* Levels at or below WARN must emit; levels above WARN must be silent. */
  CATCH_REQUIRE(capture_stdout([] { ER_FATAL("fatal at warn"); }).find("[FATAL]") != std::string::npos);
  CATCH_REQUIRE(capture_stdout([] { ER_ERR  ("error at warn"); }).find("[ERROR]") != std::string::npos);
  CATCH_REQUIRE(capture_stdout([] { ER_WARN ("warn at warn");  }).find("[WARN ]") != std::string::npos);

  CATCH_REQUIRE(capture_stdout([] { ER_INFO ("info suppressed");  }).empty());
  CATCH_REQUIRE(capture_stdout([] { ER_DEBUG("debug suppressed"); }).empty());
  CATCH_REQUIRE(capture_stdout([] { ER_TRACE("trace suppressed"); }).empty());
}

static void dprintf_available_test() {
  /* DPRINTF is available at any level >= FATAL and must emit at ERL_WARN. */
  CATCH_REQUIRE(capture_stdout([] { DPRINTF("dprintf at warn\n"); }) == "dprintf at warn\n");

  int x = 7;
  CATCH_REQUIRE(capture_stdout([&] { DPRINT_TOKD(x); }) == "x: 7\n");
}

static void suppressed_syntax_test() {
  /*
   * Suppressed macros (INFO, DEBUG, TRACE) must still compile without
   * #ifdefs – user code should not need level guards around individual calls.
   */
  ER_INFO("info nop");
  ER_DEBUG("debug nop");
  ER_TRACE("trace nop");
}

static void cond_macros_test() {
  /*
   * ER_CONDW calls ER_WARN (active at ERL_WARN) → must emit on true.
   * ER_CONDI calls ER_INFO (suppressed)          → must be silent even on true.
   * ER_CONDD calls ER_DEBUG (suppressed)         → must be silent even on true.
   */
  CATCH_REQUIRE(capture_stdout([] { ER_CONDW(true, "condw"); }).find("[WARN ]") != std::string::npos);
  CATCH_REQUIRE(capture_stdout([] { ER_CONDI(true, "condi"); }).empty());
  CATCH_REQUIRE(capture_stdout([] { ER_CONDD(true, "condd"); }).empty());
}

static void er_check_test() {
  /* ER_CHECK uses ER_ERR (active at ERL_WARN); control flow must work. */
  bool        jumped = false;
  std::string out    = capture_stdout([&] {
    ER_CHECK(false, "check at warn");
    goto done;
  error:
    jumped = true;
  done:;
  });
  CATCH_REQUIRE(jumped);
  CATCH_REQUIRE(out.find("[ERROR]")       != std::string::npos);
  CATCH_REQUIRE(out.find("check at warn") != std::string::npos);
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("ER Intermediate Emission Test", "[er][level_intermediate]") {
  emission_test();
}
CATCH_TEST_CASE("ER Intermediate DPRINTF Test", "[er][level_intermediate]") {
  dprintf_available_test();
}
CATCH_TEST_CASE("ER Intermediate Suppressed Syntax Test", "[er][level_intermediate]") {
  suppressed_syntax_test();
}
CATCH_TEST_CASE("ER Intermediate Cond Macros Test", "[er][level_intermediate]") {
  cond_macros_test();
}
CATCH_TEST_CASE("ER Intermediate Check Test", "[er][level_intermediate]") {
  er_check_test();
}
