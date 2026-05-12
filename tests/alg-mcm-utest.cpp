/**
 * \file alg-mcm-utest.cpp
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#define CATCH_CONFIG_PREFIX_ALL
#include <catch2/catch_test_macros.hpp>

#include "rcsw/algorithm/mcm_opt.h"

/*******************************************************************************
 * Test Helpers
 ******************************************************************************/
/**
 * Initialize, optimize, check min_mults and ordering, then destroy.
 * ordering_expected may be nullptr to skip the ordering check.
 */
static void check_mcm(const size_t* p,
                      size_t        size,
                      size_t        expected_min_mults,
                      const size_t* expected_ordering,
                      size_t        ordering_len) {
  struct mcm_optimizer mcm;
  CATCH_REQUIRE(OK == mcm_opt_init(&mcm, p, size));
  CATCH_REQUIRE(OK == mcm_opt_optimize(&mcm));
  CATCH_REQUIRE(mcm.min_mults == expected_min_mults);
  mcm_opt_print(&mcm);

  if (expected_ordering != nullptr) {
    std::vector<size_t> ordering(ordering_len, 0);
    CATCH_REQUIRE(OK == mcm_opt_report(&mcm, ordering.data()));
    for (size_t i = 0; i < ordering_len; ++i) {
      CATCH_REQUIRE(ordering[i] == expected_ordering[i]);
    }
  }

  mcm_opt_destroy(&mcm);
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("MCM - NULL Input Rejected", "[alg][mcm]") {
  size_t p[] = {10, 20, 30};
  struct mcm_optimizer mcm;
  CATCH_REQUIRE(ERROR == mcm_opt_init(nullptr, p, 3));
  CATCH_REQUIRE(ERROR == mcm_opt_init(&mcm, nullptr, 3));
  /* size < 2 is invalid (need at least 2 matrices = 3 dimensions) */
  CATCH_REQUIRE(ERROR == mcm_opt_init(&mcm, p, 1));
  CATCH_REQUIRE(ERROR == mcm_opt_init(&mcm, p, 0));
  mcm_opt_destroy(nullptr);  /* must not crash */
}

CATCH_TEST_CASE("MCM - Two Matrices", "[alg][mcm]") {
  /* A(10x20) * B(20x30): only one way, cost = 10*20*30 = 6000 */
  size_t p[] = {10, 20, 30};
  const size_t ord[] = {1, 2};
  check_mcm(p, 3, 6000, ord, 2);
}

CATCH_TEST_CASE("MCM - Known Cases", "[alg][mcm]") {
  CATCH_SECTION("P=[30,1,40,10,25,50,5]") {
    size_t p[] = {30, 1, 40, 10, 25, 50, 5};
    const size_t ord[] = {2, 3, 4, 5, 6, 1};
    check_mcm(p, 7, 2300, ord, 6);
  }
  CATCH_SECTION("P=[40,20,30,10,30]") {
    size_t p[] = {40, 20, 30, 10, 30};
    const size_t ord[] = {2, 3, 1, 4};
    check_mcm(p, 5, 26000, ord, 4);
  }
  CATCH_SECTION("P=[10,20,30,40,30]") {
    size_t p[] = {10, 20, 30, 40, 30};
    const size_t ord[] = {1, 2, 3, 4};
    check_mcm(p, 5, 30000, ord, 4);
  }
  CATCH_SECTION("P=[10,20,30]") {
    size_t p[] = {10, 20, 30};
    const size_t ord[] = {1, 2};
    check_mcm(p, 3, 6000, ord, 2);
  }
}

CATCH_TEST_CASE("MCM - All Same Dimensions", "[alg][mcm]") {
  /* n equal-dimension matrices: optimizer should still terminate correctly */
  size_t p[] = {10, 10, 10, 10, 10};
  struct mcm_optimizer mcm;
  CATCH_REQUIRE(OK == mcm_opt_init(&mcm, p, 5));
  CATCH_REQUIRE(OK == mcm_opt_optimize(&mcm));
  /* 4 matrices of 10x10: all orderings cost the same = 3000 */
  CATCH_REQUIRE(mcm.min_mults == 3000);
  mcm_opt_destroy(&mcm);
}

CATCH_TEST_CASE("MCM - Optimize Called Twice", "[alg][mcm]") {
  /* calling optimize twice should produce the same result */
  size_t p[] = {40, 20, 30, 10, 30};
  struct mcm_optimizer mcm;
  CATCH_REQUIRE(OK == mcm_opt_init(&mcm, p, 5));
  CATCH_REQUIRE(OK == mcm_opt_optimize(&mcm));
  size_t first = mcm.min_mults;
  CATCH_REQUIRE(OK == mcm_opt_optimize(&mcm));
  CATCH_REQUIRE(mcm.min_mults == first);
  mcm_opt_destroy(&mcm);
}

CATCH_TEST_CASE("MCM - NULL Args to Report/Print Rejected", "[alg][mcm]") {
  CATCH_REQUIRE(ERROR == mcm_opt_report(nullptr, nullptr));
  CATCH_REQUIRE(ERROR == mcm_opt_print(nullptr));
}
