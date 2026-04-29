/**
 * \file er_level_all-utest.cpp
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

/* Must be defined before macros.h sets its __FILE_NAME__ fallback. */
#define RCSW_ER_MODNAME "test.er"
#define RCSW_ER_MODID   (0x0001)

#define RCSW_CONFIG_ER_PLUGIN 1 /* RCSW_ER_PLUGIN_SIMPLE */

#include "rcsw/er/er.h"
#include "rcsw/er/plugin/simple.h"
#include "rcsw/er/macros.h"

#define CATCH_CONFIG_PREFIX_ALL
#include <catch2/catch_test_macros.hpp>

#include "tests/test_utils.hpp"

/*******************************************************************************
 * Compile-time Invariants
 *
 * Level ordering is a foundational contract; verify it once at build time.
 ******************************************************************************/
static_assert(RCSW_ERL == RCSW_ERL_ALL,       "default ERL must be ERL_ALL");
static_assert(RCSW_ERL_ALL == RCSW_ERL_TRACE,  "ERL_ALL must equal ERL_TRACE");
static_assert(RCSW_ERL_TRACE > RCSW_ERL_DEBUG, "TRACE > DEBUG");
static_assert(RCSW_ERL_DEBUG > RCSW_ERL_INFO,  "DEBUG > INFO");
static_assert(RCSW_ERL_INFO  > RCSW_ERL_WARN,  "INFO  > WARN");
static_assert(RCSW_ERL_WARN  > RCSW_ERL_ERROR, "WARN  > ERROR");
static_assert(RCSW_ERL_ERROR > RCSW_ERL_FATAL, "ERROR > FATAL");
static_assert(RCSW_ERL_FATAL > RCSW_ERL_NONE,  "FATAL > NONE");

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/

/*
 * SIMPLE plugin output format (verified experimentally):
 *
 *  FATAL  "test.er \033[31m[FATAL]\033[0m <msg>\r\n"
 *  ERROR  "test.er \033[31m[ERROR]\033[0m <msg>\r\n"
 *  WARN   "test.er [WARN ] <msg>\r\n"
 *  INFO   "test.er [INFO ] <msg>\r\n"
 *  DEBUG  "test.er [DEBUG] <msg>\r\n"
 *  TRACE  "test.er [TRACE] <msg>\r\n"
 *
 * \r\n comes from ER_REPORT appending "\r\n" to every message.
 * ANSI red codes appear only for FATAL and ERROR.
 */

static void level_output_test() {
  /* Every active level must emit its level tag and the module name. */
  for (auto& [tag, out] : {
         std::pair<const char*, std::string>{"[FATAL]", capture_stdout([] { ER_FATAL("hello %d", 42); })},
         {"[ERROR]", capture_stdout([] { ER_ERR  ("world %d", 99); })},
         {"[WARN ]", capture_stdout([] { ER_WARN ("warn msg");     })},
         {"[INFO ]", capture_stdout([] { ER_INFO ("info msg");     })},
         {"[DEBUG]", capture_stdout([] { ER_DEBUG("debug msg");    })},
         {"[TRACE]", capture_stdout([] { ER_TRACE("trace msg");    })},
       }) {
    CATCH_REQUIRE(out.find(tag)             != std::string::npos);
    CATCH_REQUIRE(out.find(RCSW_ER_MODNAME) != std::string::npos);
  }
}

static void ansi_color_test() {
  /* FATAL and ERROR carry red ANSI codes; all other levels do not. */
  std::string fatal_out = capture_stdout([] { ER_FATAL("f"); });
  std::string error_out = capture_stdout([] { ER_ERR("e");   });

  CATCH_REQUIRE(fatal_out.find(RCSW_ER_FAILC) != std::string::npos);
  CATCH_REQUIRE(fatal_out.find(RCSW_ER_ENDC)  != std::string::npos);
  CATCH_REQUIRE(error_out.find(RCSW_ER_FAILC) != std::string::npos);
  CATCH_REQUIRE(error_out.find(RCSW_ER_ENDC)  != std::string::npos);

  for (auto& out : {
         capture_stdout([] { ER_WARN("w");  }),
         capture_stdout([] { ER_INFO("i");  }),
         capture_stdout([] { ER_DEBUG("d"); }),
         capture_stdout([] { ER_TRACE("t"); }),
       }) {
    CATCH_REQUIRE(out.find(RCSW_ER_FAILC) == std::string::npos);
  }
}

static void crlf_suffix_test() {
  /* ER_REPORT appends "\r\n" to every emitted line. */
  for (auto& out : {
         capture_stdout([] { ER_FATAL("f"); }),
         capture_stdout([] { ER_ERR  ("e"); }),
         capture_stdout([] { ER_WARN ("w"); }),
         capture_stdout([] { ER_INFO ("i"); }),
         capture_stdout([] { ER_DEBUG("d"); }),
         capture_stdout([] { ER_TRACE("t"); }),
       }) {
    CATCH_REQUIRE(out.size() >= 2);
    CATCH_REQUIRE(out.substr(out.size() - 2) == "\r\n");
  }
}

static void modname_prefix_test() {
  /* The module name must appear at position 0 on every emitted line. */
  for (auto& out : {
         capture_stdout([] { ER_FATAL("f"); }),
         capture_stdout([] { ER_ERR  ("e"); }),
         capture_stdout([] { ER_WARN ("w"); }),
         capture_stdout([] { ER_INFO ("i"); }),
         capture_stdout([] { ER_DEBUG("d"); }),
         capture_stdout([] { ER_TRACE("t"); }),
       }) {
    CATCH_REQUIRE(out.find(RCSW_ER_MODNAME) == 0);
  }
}

static void dprintf_macros_test() {
  /* DPRINTF passes arguments through verbatim with no extra decoration. */
  CATCH_REQUIRE(capture_stdout([] { DPRINTF("val=%d\n", 7); }) == "val=7\n");

  /* DPRINT_TOK* stringify the token name followed by its formatted value. */
  int x = 42;
  CATCH_REQUIRE(capture_stdout([&] { DPRINT_TOKD(x); }) == "x: 42\n");
  CATCH_REQUIRE(capture_stdout([&] { DPRINT_TOKX(x); }) == "x: 0x2a\n");
  CATCH_REQUIRE(capture_stdout([&] { DPRINT_TOKF(x); }) == "x: 42.00000000\n");
  CATCH_REQUIRE(capture_stdout([&] { DPRINT_TOK(x);  }) == "x: 42/0x2a\n");
}

static void er_assert_test() {
  /* ER_ASSERT must be silent and must not abort when its condition is true. */
  CATCH_REQUIRE(capture_stdout([] { ER_ASSERT(1 == 1, "must not fire"); }).empty());

  bool reached = false;
  ER_ASSERT(true, "must not abort");
  reached = true;
  CATCH_REQUIRE(reached);

  /*
   * ER_ASSERT(false, ...) calls assert() which aborts the process; that path
   * is covered by the negative compile/process-exit tests, not here.
   */
}

static void er_cond_macros_test() {
  /* ER_CONDW/CONDI/CONDD emit when their condition is true, are silent on false. */
  CATCH_REQUIRE(capture_stdout([] { ER_CONDW(true,  "condw"); }).find("[WARN ]") != std::string::npos);
  CATCH_REQUIRE(capture_stdout([] { ER_CONDW(false, "condw"); }).empty());

  CATCH_REQUIRE(capture_stdout([] { ER_CONDI(true,  "condi"); }).find("[INFO ]") != std::string::npos);
  CATCH_REQUIRE(capture_stdout([] { ER_CONDI(false, "condi"); }).empty());

  CATCH_REQUIRE(capture_stdout([] { ER_CONDD(true,  "condd"); }).find("[DEBUG]") != std::string::npos);
  CATCH_REQUIRE(capture_stdout([] { ER_CONDD(false, "condd"); }).empty());
}

static void er_check_test() {
  /* ER_CHECK must NOT jump when its condition is true. */
  {
    bool reached = false;
    ER_CHECK(true, "no jump");
    reached = true;
    goto done_true;
  error:
    CATCH_FAIL("ER_CHECK(true) must not branch to error");
  done_true:
    CATCH_REQUIRE(reached);
  }

  /* ER_CHECK must jump to 'error' and emit an ERROR message when false. */
  {
    bool        jumped = false;
    std::string out    = capture_stdout([&] {
      ER_CHECK(false, "check failed %d", 7);
      goto done_false;
    error:
      jumped = true;
    done_false:;
    });
    CATCH_REQUIRE(jumped);
    CATCH_REQUIRE(out.find("[ERROR]")        != std::string::npos);
    CATCH_REQUIRE(out.find("check failed 7") != std::string::npos);
  }
}

static void er_sentinel_test() {
  /* ER_SENTINEL always branches to 'error' and emits an ERROR message. */
  bool        jumped = false;
  std::string out    = capture_stdout([&] {
    goto sentinel_path;
    goto done;
  sentinel_path:
    ER_SENTINEL("sentinel %d", 99);
    goto done; /* unreachable; silences fall-through warning */
  error:
    jumped = true;
  done:;
  });
  CATCH_REQUIRE(jumped);
  CATCH_REQUIRE(out.find("[ERROR]")     != std::string::npos);
  CATCH_REQUIRE(out.find("sentinel 99") != std::string::npos);
}

static void er_init_deinit_test() {
  /* SIMPLE init/deinit are no-ops; repeated calls must not crash. */
  RCSW_ER_INIT();
  RCSW_ER_INIT();
  RCSW_ER_MODULE_INIT();
  RCSW_ER_INSMOD(RCSW_ER_MODID, RCSW_ER_MODNAME);
  RCSW_ER_DEINIT();
  RCSW_ER_DEINIT();
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("ER Level Output Test", "[er][level_all]") {
  level_output_test();
}
CATCH_TEST_CASE("ER ANSI Color Test", "[er][level_all]") {
  ansi_color_test();
}
CATCH_TEST_CASE("ER CRLF Suffix Test", "[er][level_all]") {
  crlf_suffix_test();
}
CATCH_TEST_CASE("ER Modname Prefix Test", "[er][level_all]") {
  modname_prefix_test();
}
CATCH_TEST_CASE("ER DPRINTF Macros Test", "[er][level_all]") {
  dprintf_macros_test();
}
CATCH_TEST_CASE("ER Assert Test", "[er][level_all]") {
  er_assert_test();
}
CATCH_TEST_CASE("ER Conditional Macros Test", "[er][level_all]") {
  er_cond_macros_test();
}
CATCH_TEST_CASE("ER Check Test", "[er][level_all]") {
  er_check_test();
}
CATCH_TEST_CASE("ER Sentinel Test", "[er][level_all]") {
  er_sentinel_test();
}
CATCH_TEST_CASE("ER Init/Deinit Test", "[er][level_all]") {
  er_init_deinit_test();
}
