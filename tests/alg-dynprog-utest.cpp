/**
 * \file alg-dynprog-utest.cpp
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_PREFIX_ALL
#include <catch.hpp>
#include <iostream>

#include "rcsw/algorithm/mcm_opt.h"
#include "rcsw/algorithm/lcs.h"
#include "rcsw/algorithm/edit_dist.h"
#include "rcsw/algorithm/algorithm.h"

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/
static char th_multiply_cb(char x, char y) {
  if (x == 'a' && y == 'b') {
    return 'a';
  }
  return 'b';
} /* th_multiply_cb */

static bool_t th_char_cmp(const void* const e1, const void* const e2) {
  return (bool_t)(*((const char*)e1) == *((const char*)e2));
} /* th_char_cmp() */

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("Matrix Chain Optimization","[alg][Dynamic Programming]") {
  CATCH_SECTION("P = [30,1,40,10,25,50,5]") {
    size_t p[7] = {30,1,40,10,25,50,5};
    struct mcm_optimizer mcm;
    size_t ordering[7];
    CATCH_REQUIRE(OK == mcm_opt_init(&mcm,p,7));
    mcm_opt_optimize(&mcm);
    CATCH_REQUIRE(mcm.min_mults == 2300);
    mcm_opt_report(&mcm,(size_t*)&ordering);
    mcm_opt_print(&mcm);
    CATCH_REQUIRE(ordering[0] == 2);
    CATCH_REQUIRE(ordering[1] == 3);
    CATCH_REQUIRE(ordering[2] == 4);
    CATCH_REQUIRE(ordering[3] == 5);
    CATCH_REQUIRE(ordering[4] == 6);
    CATCH_REQUIRE(ordering[5] == 1);
    mcm_opt_destroy(&mcm);
  }
  CATCH_SECTION("P = [40,20,30,10,30]") {
    size_t p[5] = {40,20,30,10,30};
    size_t ordering[5];
    struct mcm_optimizer mcm;
    CATCH_REQUIRE(OK == mcm_opt_init(&mcm,p,5));
    mcm_opt_optimize(&mcm);
    CATCH_REQUIRE(mcm.min_mults == 26000);
    mcm_opt_print(&mcm);
    mcm_opt_report(&mcm,(size_t*)&ordering);
    CATCH_REQUIRE(ordering[0] == 2);
    CATCH_REQUIRE(ordering[1] == 3);
    CATCH_REQUIRE(ordering[2] == 1);
    CATCH_REQUIRE(ordering[3] == 4);
    mcm_opt_destroy(&mcm);
  }

  CATCH_SECTION("P = [10,20,30,40,30") {
    size_t p[5] = {10,20,30,40,30};
    size_t ordering[5];
    struct mcm_optimizer mcm;
    CATCH_REQUIRE(OK == mcm_opt_init(&mcm,p,5));
    mcm_opt_optimize(&mcm);
    CATCH_REQUIRE(mcm.min_mults == 30000);
    mcm_opt_print(&mcm);
    mcm_opt_report(&mcm,(size_t*)&ordering);
    CATCH_REQUIRE(ordering[0] == 1);
    CATCH_REQUIRE(ordering[1] == 2);
    CATCH_REQUIRE(ordering[2] == 3);
    CATCH_REQUIRE(ordering[3] == 4);
    mcm_opt_destroy(&mcm);
  }
  CATCH_SECTION("P = [10,20,30]") {
    size_t p[3] = {10,20,30};
    size_t ordering[3];
    struct mcm_optimizer mcm;
    CATCH_REQUIRE(OK == mcm_opt_init(&mcm,p,3));
    mcm_opt_optimize(&mcm);
    CATCH_REQUIRE(mcm.min_mults == 6000);
    mcm_opt_print(&mcm);
    mcm_opt_report(&mcm,(size_t*)&ordering);
    CATCH_REQUIRE(ordering[0] == 1);
    CATCH_REQUIRE(ordering[1] == 2);
    mcm_opt_destroy(&mcm);
  }
}

CATCH_TEST_CASE("Longest Common Subsequence","[alg][Dynamic Programming]") {
  CATCH_SECTION("X = ABCRCQ7Xz, Y = o15RCQ0Xz") {
    char x[11]={'A','B','C','R','C','Q','7','X','z','\0'};
    char y[11]={'o','1','5','R','C','Q','0','X','z','\0'};
    struct lcs_calculator lcs;
    CATCH_REQUIRE(lcs_init(NULL,x,y) == ERROR);
    CATCH_REQUIRE(lcs_init(&lcs,x,y) == OK);
    CATCH_REQUIRE(OK == lcs_iter(&lcs));
    CATCH_REQUIRE(lcs.size == 5);
    CATCH_REQUIRE(OK == lcs_rec(&lcs));
    CATCH_REQUIRE(lcs.size == 5);
    CATCH_REQUIRE(OK == strcmp(lcs.sequence,"RCQXz"));
    printf("LCS of %s and %s: %s\n",x,y,lcs.sequence);
    lcs_destroy(&lcs);
    lcs_destroy(NULL);
  }

  CATCH_SECTION("X = ABCDGH, Y = AEDFHR") {
    char x[7]={'A','B','C','D','G','H','\0'};
    char y[7]={'A','E','D','F','H','R','\0'};
    struct lcs_calculator lcs;
    lcs_init(&lcs,x,y);
    CATCH_REQUIRE(OK == lcs_iter(&lcs));
    CATCH_REQUIRE(lcs.size == 3);
    CATCH_REQUIRE(OK == lcs_rec(&lcs));
    CATCH_REQUIRE(lcs.size == 3);
    CATCH_REQUIRE(OK == strcmp(lcs.sequence,"ADH"));
    printf("LCS of %s and %s: %s\n",x,y,lcs.sequence);
    lcs_destroy(&lcs);
  }
}

CATCH_TEST_CASE("Edit Distance","[alg][Dynamic Programming]") {
  char x[10]={'A', 'B', 'C', 'R', 'C', 'Q', '7', 'X', 'z', '\0'};
  char y[10]={'o', '1', '5', 'R', 'C', 'Q', '0', 'X', 'z', '\0'};
  struct edit_dist_finder finder;

  CATCH_REQUIRE(OK == edit_dist_init(&finder, x, y, sizeof(char),
                               th_char_cmp, (size_t(*)(const void*))strlen));
  size_t res1 = edit_dist_find(&finder, EDIT_DIST_ITER);
  size_t res2 = edit_dist_find(&finder, EDIT_DIST_REC);
  CATCH_REQUIRE(res1 == 4);
  CATCH_REQUIRE(res2 == 4);
  edit_dist_destroy(&finder);
}

CATCH_TEST_CASE("Alphabet Parenthesization", "[alg][Dynamic Programming]") {
  char x1[5]={'a','b','a','b','\0'};
  char x2[5]={'b','a','a','b','\0'};
  char x3[3]={'a','b','\0'};
  char r[16];
  CATCH_REQUIRE(TRUE == str_is_parenthesizable(x1, r,'a', th_multiply_cb));
  CATCH_REQUIRE(FALSE == str_is_parenthesizable(x2, r, 'a', th_multiply_cb));
  CATCH_REQUIRE(TRUE == str_is_parenthesizable(x3, r, 'a', th_multiply_cb));
}
