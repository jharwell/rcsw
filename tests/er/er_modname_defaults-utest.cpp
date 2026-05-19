/**
 * \file er_modname_defaults-utest.cpp
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

/* Deliberately omit RCSW_ER_MODNAME / RCSW_ER_MODID to exercise fallbacks. */
#define RCSW_CONFIG_ER_PLUGIN 1

#include "rcsw/er/er.h"
#include "rcsw/er/plugin/simple.h"
#include "rcsw/er/macros.h"

#define CATCH_CONFIG_PREFIX_ALL
#include <catch2/catch_test_macros.hpp>

#include "er_test_utils.hpp"
#include <string>

/*******************************************************************************
 * Compile-time Invariants
 *
 * When the user does not define RCSW_ER_MODNAME / RCSW_ER_MODID before
 * including macros.h, the header supplies:
 *
 *   RCSW_ER_MODNAME  →  __FILE_NAME__   (compiler-provided basename)
 *   RCSW_ER_MODID    →  0xFFFFFFFF      (sentinel "unknown" ID)
 ******************************************************************************/
static_assert(RCSW_ER_MODID == 0xFFFFFFFF,
              "default RCSW_ER_MODID must be the sentinel 0xFFFFFFFF");

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/

static void default_modname_prefix_test() {
  /*
   * __FILE_NAME__ expands to the basename of this translation unit, e.g.
   * "er_modname_defaults-utest.cpp".  Every emitted line must start with it.
   */
  const std::string expected = __FILE_NAME__;

  for (auto& out : {
         capture_stdout([] { ER_FATAL("f"); }),
         capture_stdout([] { ER_ERR  ("e"); }),
         capture_stdout([] { ER_WARN ("w"); }),
         capture_stdout([] { ER_INFO ("i"); }),
         capture_stdout([] { ER_DEBUG("d"); }),
         capture_stdout([] { ER_TRACE("t"); }),
       }) {
    CATCH_REQUIRE(out.find(expected) == 0);
  }
}

static void default_modid_test() {
  /* Runtime complement to the static_assert above. */
  CATCH_REQUIRE(static_cast<unsigned>(RCSW_ER_MODID) == 0xFFFFFFFFu);
}

static void default_functionality_test() {
  /* All macros must compile and behave correctly with the default modname/id. */
  for (auto& out : {
         capture_stdout([] { ER_FATAL("f"); }),
         capture_stdout([] { ER_ERR  ("e"); }),
         capture_stdout([] { ER_WARN ("w"); }),
         capture_stdout([] { ER_INFO ("i"); }),
         capture_stdout([] { ER_DEBUG("d"); }),
         capture_stdout([] { ER_TRACE("t"); }),
       }) {
    CATCH_REQUIRE(!out.empty());
  }

  /* Module init with defaults must be a no-op. */
  RCSW_ER_MODULE_INIT();
  RCSW_ER_INSMOD(RCSW_ER_MODID, RCSW_ER_MODNAME);
}

static void default_check_flow_test() {
  /* Control-flow macros must not be affected by which modname is active. */
  ER_ASSERT(true, "must not abort");

  bool jumped = false;
  ER_CHECK(false, "check with defaults");
  goto done;
error:
  jumped = true;
done:
  CATCH_REQUIRE(jumped);
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("ER Modname Default Prefix Test", "[er][modname_defaults]") {
  default_modname_prefix_test();
}
CATCH_TEST_CASE("ER Modname Default ID Test", "[er][modname_defaults]") {
  default_modid_test();
}
CATCH_TEST_CASE("ER Modname Default Functionality Test", "[er][modname_defaults]") {
  default_functionality_test();
}
CATCH_TEST_CASE("ER Modname Default Check Flow Test", "[er][modname_defaults]") {
  default_check_flow_test();
}
