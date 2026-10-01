/**
 * \file
 *
 * \copyright 2026 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * FPC macros in ABORT mode (LIBRA_FPC=1): only the conditions are checked, so
 * functions whose failure sentinel is falsy (NULL, false, 0) must not abort
 * when their preconditions hold.
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
/* Force ABORT mode in this TU, whatever the build configured; must precede
 * every RCSW include. */
#undef LIBRA_FPC
#define LIBRA_FPC 1 /* LIBRA_FPC_ABORT */

#define CATCH_CONFIG_PREFIX_ALL
#include <catch2/catch_test_macros.hpp>

#include "rcsw/core/fpc.h"

static_assert(RCSW_FPC == RCSW_FPC_ABORT, "TU must build in FPC ABORT mode");

/*******************************************************************************
 * Helpers
 ******************************************************************************/
static void* ptr_identity(void* p) {
  RCSW_FPC_NV(nullptr, nullptr != p);
  return p;
}

static int int_identity(int x) {
  RCSW_FPC_NV(0, x > 0);
  return x;
}

static bool bool_check(int x) {
  RCSW_FPC_NV(false, x > 0, x < 100);
  return true;
}

static void void_check(int* out, int x) {
  RCSW_FPC_V(nullptr != out, x > 0);
  *out = x;
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("FPC ABORT: satisfied preconditions do not abort",
                "[core][fpc][noalloc]") {
  int x = 0;
  void_check(&x, 3);
  CATCH_REQUIRE(3 == x);
  CATCH_REQUIRE(ptr_identity(&x) == &x);
  CATCH_REQUIRE(5 == int_identity(5));
  CATCH_REQUIRE(bool_check(42));
}
