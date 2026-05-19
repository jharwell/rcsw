/**
 * \file er_plugin_simple-utest.cpp
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <string>

#define RCSW_ER_MODNAME "test.er.simple"
#define RCSW_ER_MODID   (0x0010)

#define RCSW_CONFIG_ER_PLUGIN 1 /* RCSW_ER_PLUGIN_SIMPLE */

#include "rcsw/er/er.h"
#include "rcsw/er/plugin/simple.h"
/*
 * Including plugin.h triggers a #error for every required plugin symbol that
 * is absent; if any are missing this TU will not compile.
 */
#include "rcsw/er/plugin/plugin.h"
#include "rcsw/er/macros.h"

#define CATCH_CONFIG_PREFIX_ALL
#include <catch2/catch_test_macros.hpp>

#include "er_test_utils.hpp"

/*******************************************************************************
 * Compile-time Invariants
 ******************************************************************************/

/* plugin.h #error guards passed → all required symbols are present. */
static_assert(true, "plugin interface guards passed");

/* SIMPLE joins hierarchical module name components with '.'. */
static_assert(RCSW_ER_PLUGIN_MODNAME_COMPONENT_SEPARATOR[0] == '.',
              "SIMPLE plugin must use '.' as component separator");

/* LVL_CHECK is a pure constant expression for SIMPLE. */
static_assert(RCSW_ER_PLUGIN_LVL_CHECK(nullptr, FATAL), "FATAL enabled at ERL_ALL");
static_assert(RCSW_ER_PLUGIN_LVL_CHECK(nullptr, TRACE), "TRACE enabled at ERL_ALL");

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/

static void interface_nop_test() {
  /* SIMPLE init/deinit/insmod are no-ops and must not crash. */
  RCSW_ER_PLUGIN_INIT();
  RCSW_ER_PLUGIN_DEINIT();
  RCSW_ER_PLUGIN_INSMOD(RCSW_ER_MODID, RCSW_ER_MODNAME);
}

static void lvl_check_test() {
  /* At ERL_ALL every level check must return true. */
  CATCH_REQUIRE(RCSW_ER_PLUGIN_LVL_CHECK(nullptr, FATAL));
  CATCH_REQUIRE(RCSW_ER_PLUGIN_LVL_CHECK(nullptr, ERROR));
  CATCH_REQUIRE(RCSW_ER_PLUGIN_LVL_CHECK(nullptr, WARN));
  CATCH_REQUIRE(RCSW_ER_PLUGIN_LVL_CHECK(nullptr, INFO));
  CATCH_REQUIRE(RCSW_ER_PLUGIN_LVL_CHECK(nullptr, DEBUG));
  CATCH_REQUIRE(RCSW_ER_PLUGIN_LVL_CHECK(nullptr, TRACE));
}

static void report_format_test() {
  /*
   * Call RCSW_ER_PLUGIN_REPORT directly to test the raw formatting path,
   * bypassing the level-gating wrappers in the ER_* macros.
   *
   * Expected format per level:
   *   FATAL/ERROR  "MODNAME \033[31m[LVL]\033[0m msg\r\n"
   *   WARN+        "MODNAME [LVL_TAG] msg\r\n"
   */
  struct {
    const char* tag;
    bool        has_ansi;
    std::string out;
  } cases[] = {
    { "[FATAL]", true,  capture_stdout([] {
        RCSW_ER_PLUGIN_REPORT(FATAL, nullptr, RCSW_ER_MODID, RCSW_ER_MODNAME, "fp\r\n"); }) },
    { "[ERROR]", true,  capture_stdout([] {
        RCSW_ER_PLUGIN_REPORT(ERROR, nullptr, RCSW_ER_MODID, RCSW_ER_MODNAME, "ep\r\n"); }) },
    { "[WARN ]", false, capture_stdout([] {
        RCSW_ER_PLUGIN_REPORT(WARN,  nullptr, RCSW_ER_MODID, RCSW_ER_MODNAME, "wp\r\n"); }) },
    { "[INFO ]", false, capture_stdout([] {
        RCSW_ER_PLUGIN_REPORT(INFO,  nullptr, RCSW_ER_MODID, RCSW_ER_MODNAME, "ip\r\n"); }) },
    { "[DEBUG]", false, capture_stdout([] {
        RCSW_ER_PLUGIN_REPORT(DEBUG, nullptr, RCSW_ER_MODID, RCSW_ER_MODNAME, "dp\r\n"); }) },
    { "[TRACE]", false, capture_stdout([] {
        RCSW_ER_PLUGIN_REPORT(TRACE, nullptr, RCSW_ER_MODID, RCSW_ER_MODNAME, "tp\r\n"); }) },
  };

  for (auto& c : cases) {
    /* Module name must be the first token. */
    CATCH_REQUIRE(c.out.find(RCSW_ER_MODNAME) == 0);

    /* Level tag must appear. */
    CATCH_REQUIRE(c.out.find(c.tag) != std::string::npos);

    /* ANSI red codes present for FATAL/ERROR, absent for all others. */
    if (c.has_ansi) {
      CATCH_REQUIRE(c.out.find(RCSW_ER_FAILC) != std::string::npos);
    } else {
      CATCH_REQUIRE(c.out.find(RCSW_ER_FAILC) == std::string::npos);
    }
  }
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("ER Simple Plugin Interface Test", "[er][plugin_simple]") {
  interface_nop_test();
}
CATCH_TEST_CASE("ER Simple Plugin LvlCheck Test", "[er][plugin_simple]") {
  lvl_check_test();
}
CATCH_TEST_CASE("ER Simple Plugin Report Format Test", "[er][plugin_simple]") {
  report_format_test();
}
