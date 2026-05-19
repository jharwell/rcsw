/**
 * \file er_level_none-utest.cpp
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#define LIBRA_ERL 0 /* RCSW_ERL_NONE */

#define RCSW_ER_MODNAME "test.er.none"
#define RCSW_ER_MODID   (0x0002)

#define RCSW_CONFIG_ER_PLUGIN 1

#include "rcsw/er/er.h"
#include "rcsw/er/plugin/simple.h"
#include "rcsw/er/macros.h"

#define CATCH_CONFIG_PREFIX_ALL
#include <catch2/catch_test_macros.hpp>

#include "er_test_utils.hpp"

/*******************************************************************************
 * Compile-time Invariants
 ******************************************************************************/
static_assert(RCSW_ERL == RCSW_ERL_NONE, "ERL must be NONE in this TU");

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/

static void silence_test() {
  /* Every ER level macro must produce no output whatsoever at ERL_NONE. */
  for (auto& out : {
         capture_stdout([] { ER_FATAL("must not appear"); }),
         capture_stdout([] { ER_ERR  ("must not appear"); }),
         capture_stdout([] { ER_WARN ("must not appear"); }),
         capture_stdout([] { ER_INFO ("must not appear"); }),
         capture_stdout([] { ER_DEBUG("must not appear"); }),
         capture_stdout([] { ER_TRACE("must not appear"); }),
       }) {
    CATCH_REQUIRE(out.empty());
  }
}

static void dprintf_silence_test() {
  /*
   * DPRINTF and DPRINT_TOK* are gated by ERL >= FATAL; at ERL_NONE they
   * must expand to nothing.
   */
  int x = 42;
  std::string out = capture_stdout([&] {
    DPRINTF("must not appear\n");
    DPRINT_TOKD(x);
    DPRINT_TOKX(x);
    DPRINT_TOKF(x);
    DPRINT_TOK(x);
  });
  CATCH_REQUIRE(out.empty());
}

static void syntax_test() {
  /*
   * Every macro must remain syntactically legal when it expands to nothing
   * so that user code needs no #ifdefs around individual log calls.
   * Reaching this point without a compile error proves the contract.
   */
  ER_FATAL("f"); ER_ERR("e"); ER_WARN("w");
  ER_INFO("i");  ER_DEBUG("d"); ER_TRACE("t");
  int x = 1;
  DPRINTF("dp\n"); DPRINT_TOKD(x); DPRINT_TOKX(x); DPRINT_TOKF(x); DPRINT_TOK(x);

  RCSW_ER_INIT();
  RCSW_ER_MODULE_INIT();
  RCSW_ER_INSMOD(RCSW_ER_MODID, RCSW_ER_MODNAME);
  RCSW_ER_DEINIT();
}

static void er_assert_still_guards_test() {
  /* ER_ASSERT is always present regardless of level; must not abort on true. */
  ER_ASSERT(true, "must not abort");

  bool reached = false;
  ER_ASSERT(1 == 1, "must not abort");
  reached = true;
  CATCH_REQUIRE(reached);
}

static void er_check_still_flows_test() {
  /* ER_CHECK control flow must work even at ERL_NONE. */
  bool reached = false;
  ER_CHECK(true, "no jump");
  reached = true;
  goto done;
error:
  CATCH_FAIL("ER_CHECK(true) must not branch to error");
done:
  CATCH_REQUIRE(reached);
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("ER None Silence Test", "[er][level_none]") {
  silence_test();
}
CATCH_TEST_CASE("ER None DPRINTF Silence Test", "[er][level_none]") {
  dprintf_silence_test();
}
CATCH_TEST_CASE("ER None Syntax Test", "[er][level_none]") {
  syntax_test();
}
CATCH_TEST_CASE("ER None Assert Test", "[er][level_none]") {
  er_assert_still_guards_test();
}
CATCH_TEST_CASE("ER None Check Flow Test", "[er][level_none]") {
  er_check_still_flows_test();
}
