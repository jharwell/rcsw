/**
 * \file
 *
 * \copyright 2026 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * The ER and RCSW_CHECK macros are single statements: they can be the
 * unbraced body of an if/else, and ER_ASSERT() evaluates its condition exactly
 * once.
 *
 * NDEBUG is defined so that a failing ER_ASSERT() reports without aborting.
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#define NDEBUG

#undef LIBRA_ERL
#define LIBRA_ERL 6 /* RCSW_ERL_ALL */

#define RCSW_ER_MODNAME "test.er.stmt"
#define RCSW_ER_MODID (0x0003)

#define RCSW_CONFIG_ER_PLUGIN 1 /* RCSW_ER_PLUGIN_SIMPLE */

#include "rcsw/core/core.h"
#include "rcsw/er/er.h"
#include "rcsw/er/macros.h"
#include "rcsw/er/plugin/simple.h"

#define CATCH_CONFIG_PREFIX_ALL
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "tests/unit/test_utils.hpp"

static_assert(RCSW_ERL == RCSW_ERL_ALL, "ERL must be ALL in this TU");

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/
namespace {
int g_evals = 0;

int counted(int r) {
  ++g_evals;
  return r;
}

/*
 * Every macro below is the unbraced body of an if or an else. With a macro
 * that expands to a braced block or to several statements, the "; else" would
 * not compile, or the else would bind to the wrong if.
 */
status_t unbraced(int x) {
  if (x > 0)
    RCSW_CHECK(x < 10);
  else
    RCSW_CHECK_PTR(NULL);

  if (x > 5)
    ER_CHECK(x < 8, "x=%d", x);
  else
    ER_ERR("small x=%d", x);

  if (x == 9)
    ER_SENTINEL("nine");
  else
    ER_DEBUG("not nine");

  if (x == 7)
    ER_WARN("seven");
  else
    ER_INFO("not seven");

  if (x == 3)
    ER_CONDW(x == 3, "three");
  else
    ER_TRACE("not three");

  if (x == 4)
    DPRINTF("four\n");
  else
    DPRINTF("x=%d\n", x);

  if (x == 2)
    ER_ASSERT(x == 2, "two");
  else
    ER_FATAL("not two: %d", x);

  return OK;

error:
  return ERROR;
}
} /* namespace */

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("ER and RCSW_CHECK macros work as unbraced if/else bodies",
                "[er][noalloc]") {
  std::vector<status_t> rc;
  capture_stdout([&] {
    for (int x : {1, 2, 7, -1, 8, 9}) {
      rc.push_back(unbraced(x));
    }
  });
  /* -1: RCSW_CHECK_PTR(NULL), 8: ER_CHECK(x < 8), 9: ER_SENTINEL */
  CATCH_REQUIRE(std::vector<status_t>({OK, OK, OK, ERROR, ERROR, ERROR}) == rc);
}

CATCH_TEST_CASE("ER_ASSERT evaluates its condition exactly once",
                "[er][noalloc]") {
  g_evals = 0;
  ER_ASSERT(counted(1), "never reported");
  CATCH_REQUIRE(1 == g_evals);

  g_evals         = 0;
  std::string out = capture_stdout(
    [] { ER_ASSERT(counted(0), "reported once"); });
  CATCH_REQUIRE(1 == g_evals);
  CATCH_REQUIRE(out.find("reported once") != std::string::npos);
}
