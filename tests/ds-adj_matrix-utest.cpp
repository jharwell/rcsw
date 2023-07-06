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
#include <limits.h>
#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_PREFIX_ALL
#include <catch.hpp>

#include "rcsw/ds/adj_matrix.h"
#include "tests/ds_test.h"
#include "tests/ds_test.hpp"

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/
template<typename T>
void run_test(void (*test)(struct adj_matrix_params *params)) {
  /* dbg_init(); */
  /* dbg_insmod(M_TESTING, "Testing"); */
  /* dbg_insmod(M_DS_STATIC_ADJ_MATRIX, "STATIC_adj_matrix"); */
  /* dbg_mod_lvl_set(M_DS_STATIC_ADJ_MATRIX, DBG_V); */
  struct adj_matrix_params params;
  params.flags = 0;
  params.elt_size = sizeof(T);

  uint32_t flags[] = {
    RCSW_NOALLOC_HANDLE,
    RCSW_NOALLOC_DATA,
  };

  CATCH_REQUIRE(th_ds_init(&params) == OK);
  for (size_t m = 0; m < 2; ++m) {
    for (size_t k = 0; k < 2; ++k) {
      for (size_t i = 0; i < RCSW_ARRAY_ELTS(flags); ++i) {
        /* cannot have matrix that is weighted but undirected */
        if (k && !m) {
          continue;
        }
        params.is_directed = (bool_t)m;
        params.is_weighted = (bool_t)k;
        params.n_vertices = TH_NUM_ITEMS;
        params.flags = flags[i];

        test(&params);
      } /* for(i..) */
    } /* for(k..) */
  } /* for(m..) */
  th_ds_shutdown(&params);
} /* run_test() */

/*******************************************************************************
 * Test Functions
 ******************************************************************************/
template<typename T>
static void edge_add_test(struct adj_matrix_params* params) {
  struct adj_matrix *matrix;
  struct adj_matrix mymatrix;

  matrix = adj_matrix_init(&mymatrix, params);
  CATCH_REQUIRE(nullptr != matrix);

  for (size_t i = 1; i < params->n_vertices; ++i) {
    if (matrix->is_directed) {
      double val = rand() % 10 + 1;
      CATCH_REQUIRE(OK == adj_matrix_edge_addd(matrix, i-1, i, &val));
    } else {
      CATCH_REQUIRE(OK == adj_matrix_edge_addu(matrix, i-1, i));
    }
    CATCH_REQUIRE(true == adj_matrix_edge_query(matrix, i-1, i));
    if (!matrix->is_directed) {
      CATCH_REQUIRE(true == adj_matrix_edge_query(matrix, i, i-1));
    }
  } /* for(i..) */
  adj_matrix_destroy(matrix);
}

template<typename T>
static void edge_remove_test(struct adj_matrix_params* params) {
  struct adj_matrix *matrix;
  struct adj_matrix mymatrix;

  matrix = adj_matrix_init(&mymatrix, params);
  CATCH_REQUIRE(nullptr != matrix);

  size_t max = params->n_vertices;
  for (size_t i = 1; i < max; ++i) {
    if (matrix->is_directed) {
      double val = rand() % 10 + 1;
      CATCH_REQUIRE(OK == adj_matrix_edge_addd(matrix, i-1, i, &val));
    } else {
      CATCH_REQUIRE(OK == adj_matrix_edge_addu(matrix, i-1, i));
    }
    CATCH_REQUIRE(true == adj_matrix_edge_query(matrix, i-1, i));
    if (!matrix->is_directed) {
      CATCH_REQUIRE(true == adj_matrix_edge_query(matrix, i, i-1));
    }
  } /* for(i..) */


  while (!adj_matrix_isempty(matrix)) {
    size_t u = rand() % max;
    size_t v = rand() % max;
    if (adj_matrix_edge_query(matrix, u, v)) {
      CATCH_REQUIRE(OK == adj_matrix_edge_remove(matrix, u, v));
      CATCH_REQUIRE(false == adj_matrix_edge_query(matrix, u, v));
      if (!matrix->is_directed) {
        CATCH_REQUIRE(false == adj_matrix_edge_query(matrix, v, u));
      }
    }
  } /* while() */

  adj_matrix_destroy(matrix);
}

template<typename T>
static void transpose_test(struct adj_matrix_params* params) {
  struct adj_matrix *matrix;
  struct adj_matrix mymatrix;

  matrix = adj_matrix_init(&mymatrix, params);
  CATCH_REQUIRE(nullptr != matrix);

  size_t max = params->n_vertices;
  for (size_t i = 1; i < max; ++i) {
    if (matrix->is_directed) {
      double val = rand() % 10 + 1;
      CATCH_REQUIRE(OK == adj_matrix_edge_addd(matrix, i-1, i, &val));
    } else {
      CATCH_REQUIRE(OK == adj_matrix_edge_addu(matrix, i-1, i));
    }
  } /* for(i..) */

  CATCH_REQUIRE(OK == adj_matrix_transpose(matrix));

  for (size_t i = 1; i < max; ++i) {
    CATCH_REQUIRE(true == adj_matrix_edge_query(matrix, i, i-1));
  } /* for(i..) */

  adj_matrix_destroy(matrix);
}
template<typename T>
static void print_test(struct adj_matrix_params* params) {
  struct adj_matrix *matrix;
  struct adj_matrix mymatrix;

  adj_matrix_print(NULL);
  matrix = adj_matrix_init(&mymatrix, params);
  CATCH_REQUIRE(nullptr != matrix);
  adj_matrix_print(matrix);

  size_t max = params->n_vertices;
  for (size_t i = 1; i < max; ++i) {
    if (matrix->is_directed) {
      double val = rand() % 10 + 1;
      CATCH_REQUIRE(OK == adj_matrix_edge_addd(matrix, i-1, i, &val));
    } else {
      CATCH_REQUIRE(OK == adj_matrix_edge_addu(matrix, i-1, i));
    }
  } /* for(i..) */

  adj_matrix_print(matrix);
  adj_matrix_destroy(matrix);
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("Edge Add Test", "[ds][adj_matrix]") {
  run_test<element8>(edge_add_test<element8>);
  run_test<element4>(edge_add_test<element4>);
  run_test<element2>(edge_add_test<element2>);
  run_test<element1>(edge_add_test<element1>);
}
CATCH_TEST_CASE("Edge Remove Test", "[ds][adj_matrix]") {
  run_test<element8>(edge_remove_test<element8>);
  run_test<element4>(edge_remove_test<element4>);
  run_test<element2>(edge_remove_test<element2>);
  run_test<element1>(edge_remove_test<element1>);
}
CATCH_TEST_CASE("Transpose Test", "[ds][adj_matrix]") {
  run_test<element8>(transpose_test<element8>);
  run_test<element4>(transpose_test<element4>);
  run_test<element2>(transpose_test<element2>);
  run_test<element1>(transpose_test<element1>);
}

CATCH_TEST_CASE("Print Test", "[ds][adj_matrix]") {
  run_test<element8>(print_test<element8>);
  run_test<element4>(print_test<element4>);
  run_test<element2>(print_test<element2>);
  run_test<element1>(print_test<element1>);
}
