/**
 * \file ds-adj_matrix-test.cpp
 *
 * Test of adjacency matrix (static only at the moment...)
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdlib.h>
#include <limits.h>
#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_PREFIX_ALL
#include <catch.hpp>

extern "C" {
  /*
   * isnan() is in the std namespace in C++--need to bring it to global
   * namespace to test C code
   */
#include <cmath>
using std::isnan;
}

#include "rcsw/ds/static_adj_matrix.h"
#include "rcsw/common/dbg.h"
#include "tests/ds_test.h"

/*******************************************************************************
 * Test Functions
 ******************************************************************************/
/* test functions */
void static_edge_add_test(struct ds_params* params);
void static_edge_remove_test(struct ds_params* params);
void static_transpose_test(struct ds_params* params);
void test_runner(void (*test)(struct ds_params *params));

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/
void test_runner(void (*test)(struct ds_params *params)) {
  dbg_init();
  dbg_insmod(M_TESTING, "Testing");
  dbg_insmod(M_DS_STATIC_ADJ_MATRIX, "STATIC_adj_matrix");
  dbg_mod_lvl_set(M_DS_STATIC_ADJ_MATRIX, DBG_V);
  struct ds_params params;
  params.tag = DS_ADJ_MATRIX;
  params.flags = 0;
  params.el_size = sizeof(struct element);

  for (size_t m = 0; m < 2; ++m) {
    params.type.adjm.is_directed = (bool_t)m;
    for (size_t k = 0; k < 2; ++k) {
      params.type.adjm.is_weighted = (bool_t)k;
      params.type.adjm.n_vertices = NUM_ITEMS;

      params.flags = DS_APP_DOMAIN_HANDLE;

      /* cannot have matrix that is weighted but undirected */
      if (k && !m) {
        continue;
      }
      test(&params);

      CATCH_REQUIRE(th_ds_init(&params) == OK);
      params.flags = DS_APP_DOMAIN_DATA;
      test(&params);
      th_ds_shutdown(&params);
    } /* for(k..) */
  } /* for(m..) */
} /* test_runner() */

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("Static Edge Add Test", "[static_adj_matrix]") { test_runner(static_edge_add_test); }
CATCH_TEST_CASE("Static Edge Remove Test", "[static_adj_matrix]") { test_runner(static_edge_remove_test); }
CATCH_TEST_CASE("Static Transpose Test", "[static_adj_matrix]") { test_runner(static_transpose_test); }

void static_edge_add_test(struct ds_params* params) {
  struct static_adj_matrix *matrix;
  struct static_adj_matrix mymatrix;

  matrix = static_adj_matrix_init(&mymatrix, params);
  CATCH_REQUIRE(NULL != matrix);
  for (size_t i = 1; i < params->type.adjm.n_vertices; ++i) {
    if (matrix->is_directed) {
      double val = rand() % 10 + 1;
      CATCH_REQUIRE(OK == static_adj_matrix_edge_addd(matrix, i-1, i, &val));
    } else {
      CATCH_REQUIRE(OK == static_adj_matrix_edge_addu(matrix, i-1, i));
    }
    CATCH_REQUIRE(TRUE == static_adj_matrix_edge_query(matrix, i-1, i));
    if (!matrix->is_directed) {
      CATCH_REQUIRE(TRUE == static_adj_matrix_edge_query(matrix, i, i-1));
    }
  } /* for(i..) */
  static_adj_matrix_destroy(matrix);
}
void static_edge_remove_test(struct ds_params* params) {
  struct static_adj_matrix *matrix;
  struct static_adj_matrix mymatrix;

  matrix = static_adj_matrix_init(&mymatrix, params);
  CATCH_REQUIRE(NULL != matrix);

  size_t max = params->type.adjm.n_vertices;
  for (size_t i = 1; i < max; ++i) {
    if (matrix->is_directed) {
      double val = rand() % 10 + 1;
      CATCH_REQUIRE(OK == static_adj_matrix_edge_addd(matrix, i-1, i, &val));
    } else {
      CATCH_REQUIRE(OK == static_adj_matrix_edge_addu(matrix, i-1, i));
    }
    CATCH_REQUIRE(TRUE == static_adj_matrix_edge_query(matrix, i-1, i));
    if (!matrix->is_directed) {
      CATCH_REQUIRE(TRUE == static_adj_matrix_edge_query(matrix, i, i-1));
    }
  } /* for(i..) */


  while (!static_adj_matrix_isempty(matrix)) {
    size_t u = rand() % max;
    size_t v = rand() % max;
    if (static_adj_matrix_edge_query(matrix, u, v)) {
      CATCH_REQUIRE(OK == static_adj_matrix_edge_remove(matrix, u, v));
      CATCH_REQUIRE(FALSE == static_adj_matrix_edge_query(matrix, u, v));
      if (!matrix->is_directed) {
        CATCH_REQUIRE(FALSE == static_adj_matrix_edge_query(matrix, v, u));
      }
    }
  } /* while() */

  static_adj_matrix_destroy(matrix);
}
void static_transpose_test(struct ds_params* params) {
  struct static_adj_matrix *matrix;
  struct static_adj_matrix mymatrix;

  matrix = static_adj_matrix_init(&mymatrix, params);
  CATCH_REQUIRE(NULL != matrix);

  size_t max = params->type.adjm.n_vertices;
  for (size_t i = 1; i < max; ++i) {
    if (matrix->is_directed) {
      double val = rand() % 10 + 1;
      CATCH_REQUIRE(OK == static_adj_matrix_edge_addd(matrix, i-1, i, &val));
    } else {
      CATCH_REQUIRE(OK == static_adj_matrix_edge_addu(matrix, i-1, i));
    }
  } /* for(i..) */

  CATCH_REQUIRE(OK == static_adj_matrix_transpose(matrix));

  for (size_t i = 1; i < max; ++i) {
    CATCH_REQUIRE(TRUE == static_adj_matrix_edge_query(matrix, i, i-1));
  } /* for(i..) */

  static_adj_matrix_destroy(matrix);
}
