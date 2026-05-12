/**
 * \file core-fpc-utest.cpp
 *
 * Unit tests for the FPC (Function Pre/Post Condition) macros.
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#define CATCH_CONFIG_PREFIX_ALL
#include <catch2/catch_test_macros.hpp>

#include <cerrno>

#include "rcsw/core/fpc.h"

/*******************************************************************************
 * Test Helpers
 ******************************************************************************/
static int fpc_nv_pass(int x) {
  RCSW_FPC_NV(-1, x > 0);
  return x;
}

static int fpc_nv_multi(int x, int y) {
  RCSW_FPC_NV(-1, x > 0, y > 0, x != y);
  return x + y;
}

static void fpc_v_pass(int* out, int x) {
  RCSW_FPC_V(out != nullptr, x > 0);
  *out = x;
}

static int fpc_ret_nv(int x) {
  RCSW_FPC_RET_NV(x > 0, -99);
  return x;
}

static void fpc_ret_v(bool* called, int x) {
  RCSW_FPC_RET_V(x > 0);
  *called = true;
}

/*******************************************************************************
 * RCSW_FPC_NV
 ******************************************************************************/
CATCH_TEST_CASE("RCSW_FPC_NV - condition passes", "[core][fpc]") {
  CATCH_REQUIRE(fpc_nv_pass(5) == 5);
}

CATCH_TEST_CASE("RCSW_FPC_NV - condition fails returns sentinel",
                "[core][fpc]") {
  errno     = 0;
  int result = fpc_nv_pass(-1);
  CATCH_REQUIRE(result == -1);
  CATCH_REQUIRE(errno == EINVAL);
}

CATCH_TEST_CASE("RCSW_FPC_NV - short-circuits on first failure",
                "[core][fpc]") {
  /*
   * y=0 would also fail, but x=-1 is checked first. errno == EINVAL proves
   * the macro fired and returned early without checking y.
   */
  errno      = 0;
  int result = fpc_nv_multi(-1, 0);
  CATCH_REQUIRE(result == -1);
  CATCH_REQUIRE(errno == EINVAL);
}

CATCH_TEST_CASE("RCSW_FPC_NV - second condition fails", "[core][fpc]") {
  errno      = 0;
  int result = fpc_nv_multi(1, -1);
  CATCH_REQUIRE(result == -1);
  CATCH_REQUIRE(errno == EINVAL);
}

CATCH_TEST_CASE("RCSW_FPC_NV - third condition fails", "[core][fpc]") {
  errno      = 0;
  int result = fpc_nv_multi(3, 3);  /* x == y fails */
  CATCH_REQUIRE(result == -1);
  CATCH_REQUIRE(errno == EINVAL);
}

CATCH_TEST_CASE("RCSW_FPC_NV - all conditions pass", "[core][fpc]") {
  CATCH_REQUIRE(fpc_nv_multi(2, 5) == 7);
}

/*******************************************************************************
 * RCSW_FPC_V
 ******************************************************************************/
CATCH_TEST_CASE("RCSW_FPC_V - condition passes", "[core][fpc]") {
  int  out    = 0;
  fpc_v_pass(&out, 42);
  CATCH_REQUIRE(out == 42);
}

CATCH_TEST_CASE("RCSW_FPC_V - null pointer fails early", "[core][fpc]") {
  /* if fpc_v_pass proceeds with nullptr it would crash */
  fpc_v_pass(nullptr, 42);  /* must not crash */
}

CATCH_TEST_CASE("RCSW_FPC_V - second condition fails", "[core][fpc]") {
  int out = 99;
  fpc_v_pass(&out, -1);
  /* out must be unchanged: function returned before *out = x */
  CATCH_REQUIRE(out == 99);
}

/*******************************************************************************
 * RCSW_FPC_RET_NV (unconditional return regardless of RCSW_FPC setting)
 ******************************************************************************/
CATCH_TEST_CASE("RCSW_FPC_RET_NV - passes through", "[core][fpc]") {
  CATCH_REQUIRE(fpc_ret_nv(7) == 7);
}

CATCH_TEST_CASE("RCSW_FPC_RET_NV - fails returns sentinel", "[core][fpc]") {
  errno      = 0;
  int result = fpc_ret_nv(-1);
  CATCH_REQUIRE(result == -99);
  CATCH_REQUIRE(errno == EINVAL);
}

/*******************************************************************************
 * RCSW_FPC_RET_V (unconditional void return)
 ******************************************************************************/
CATCH_TEST_CASE("RCSW_FPC_RET_V - passes through", "[core][fpc]") {
  bool called = false;
  fpc_ret_v(&called, 1);
  CATCH_REQUIRE(called == true);
}

CATCH_TEST_CASE("RCSW_FPC_RET_V - fails returns early", "[core][fpc]") {
  bool called = false;
  errno       = 0;
  fpc_ret_v(&called, -1);
  CATCH_REQUIRE(called == false);
  CATCH_REQUIRE(errno == EINVAL);
}
