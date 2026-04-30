/**
 * \file ds-adjmatrix-test.cpp
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
#define CATCH_CONFIG_PREFIX_ALL
#include <catch2/catch_test_macros.hpp>

#include "rcsw/ds/adjmatrix.h"
#include "tests/ds_test.h"
#include "tests/ds_test.hpp"

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/
template <typename T>
void run_test(void (*test)(struct adjmatrix_config *config)) {
  struct adjmatrix_config config;
  memset(&config, 0, sizeof(adjmatrix_config));
  config.flags    = 0;
  config.elt_size = sizeof(T);
  CATCH_REQUIRE(th::ds_init(&config) == OK);

  uint32_t flags[] = {
    RCSW_NONE,
    RCSW_ZALLOC,
    RCSW_NOALLOC_HANDLE,
    RCSW_NOALLOC_DATA,
  };

  uint32_t applied = 0;
  for (size_t i = 0; i < RCSW_ARRAY_ELTS(flags); ++i) {
    applied |= flags[i];
    for (size_t j = i + 1; j < RCSW_ARRAY_ELTS(flags); ++j) {
      applied |= flags[j];

      for (size_t m = 0; m < 2; ++m) {
        for (size_t k = 0; k < 2; ++k) {
          /* cannot have matrix that is weighted but undirected */
          if (k && !m) {
            continue;
          }
          config.is_directed = (bool_t)m;
          config.is_weighted = (bool_t)k;
          config.n_vertices  = TH_NUM_ITEMS;
          config.flags       = applied;

          test(&config);
        } /* for(k..) */
      } /* for(m..) */

      applied &= ~flags[j];
    } /* for(j..) */
  } /* for(i..) */

  th::ds_shutdown(&config);
} /* run_test() */

/*******************************************************************************
 * Test Functions
 ******************************************************************************/
template <typename T>
static void edge_add_test(struct adjmatrix_config *config) {
  struct adjmatrix *matrix;
  struct adjmatrix  mymatrix;

  matrix = adjmatrix_init(&mymatrix, config);
  CATCH_REQUIRE(nullptr != matrix);

  for (size_t i = 1; i < config->n_vertices; ++i) {
    if (matrix->is_directed) {
      double val = rand() % 10 + 1;
      CATCH_REQUIRE(OK == adjmatrix_edge_addd(matrix, i - 1, i, &val));
    } else {
      CATCH_REQUIRE(OK == adjmatrix_edge_addu(matrix, i - 1, i));
    }
    CATCH_REQUIRE(true == adjmatrix_edge_query(matrix, i - 1, i));
    if (!matrix->is_directed) {
      CATCH_REQUIRE(true == adjmatrix_edge_query(matrix, i, i - 1));
    }
  } /* for(i..) */
  adjmatrix_destroy(matrix);
}

template <typename T>
static void edge_remove_test(struct adjmatrix_config *config) {
  struct adjmatrix *matrix;
  struct adjmatrix  mymatrix;

  matrix = adjmatrix_init(&mymatrix, config);
  CATCH_REQUIRE(nullptr != matrix);

  size_t max = config->n_vertices;
  for (size_t i = 1; i < max; ++i) {
    if (matrix->is_directed) {
      double val = rand() % 10 + 1;
      CATCH_REQUIRE(OK == adjmatrix_edge_addd(matrix, i - 1, i, &val));
    } else {
      CATCH_REQUIRE(OK == adjmatrix_edge_addu(matrix, i - 1, i));
    }
    CATCH_REQUIRE(true == adjmatrix_edge_query(matrix, i - 1, i));
    if (!matrix->is_directed) {
      CATCH_REQUIRE(true == adjmatrix_edge_query(matrix, i, i - 1));
    }
  } /* for(i..) */

  while (!adjmatrix_isempty(matrix)) {
    size_t u = rand() % max;
    size_t v = rand() % max;
    if (adjmatrix_edge_query(matrix, u, v)) {
      CATCH_REQUIRE(OK == adjmatrix_edge_remove(matrix, u, v));
      CATCH_REQUIRE(false == adjmatrix_edge_query(matrix, u, v));
      if (!matrix->is_directed) {
        CATCH_REQUIRE(false == adjmatrix_edge_query(matrix, v, u));
      }
    }
  } /* while() */

  adjmatrix_destroy(matrix);
}

template <typename T>
static void transpose_test(struct adjmatrix_config *config) {
  struct adjmatrix *matrix;
  struct adjmatrix  mymatrix;

  matrix = adjmatrix_init(&mymatrix, config);
  CATCH_REQUIRE(nullptr != matrix);

  size_t max = config->n_vertices;
  for (size_t i = 1; i < max; ++i) {
    if (matrix->is_directed) {
      double val = rand() % 10 + 1;
      CATCH_REQUIRE(OK == adjmatrix_edge_addd(matrix, i - 1, i, &val));
    } else {
      CATCH_REQUIRE(OK == adjmatrix_edge_addu(matrix, i - 1, i));
    }
  } /* for(i..) */

  CATCH_REQUIRE(OK == adjmatrix_transpose(matrix));

  for (size_t i = 1; i < max; ++i) {
    CATCH_REQUIRE(true == adjmatrix_edge_query(matrix, i, i - 1));
  } /* for(i..) */

  adjmatrix_destroy(matrix);
}
template <typename T>
static void print_test(struct adjmatrix_config *config) {
  struct adjmatrix *matrix;
  struct adjmatrix  mymatrix;

  adjmatrix_print(NULL);
  matrix = adjmatrix_init(&mymatrix, config);
  CATCH_REQUIRE(nullptr != matrix);
  adjmatrix_print(matrix);

  size_t max = config->n_vertices;
  for (size_t i = 1; i < max; ++i) {
    if (matrix->is_directed) {
      double val = rand() % 10 + 1;
      CATCH_REQUIRE(OK == adjmatrix_edge_addd(matrix, i - 1, i, &val));
    } else {
      CATCH_REQUIRE(OK == adjmatrix_edge_addu(matrix, i - 1, i));
    }
  } /* for(i..) */

  adjmatrix_print(matrix);
  adjmatrix_destroy(matrix);
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("Edge Add Test", "[ds][adjmatrix]") {
  run_test<element8>(edge_add_test<element8>);
  run_test<element4>(edge_add_test<element4>);
  run_test<element2>(edge_add_test<element2>);
  run_test<element1>(edge_add_test<element1>);
}
CATCH_TEST_CASE("Edge Remove Test", "[ds][adjmatrix]") {
  run_test<element8>(edge_remove_test<element8>);
  run_test<element4>(edge_remove_test<element4>);
  run_test<element2>(edge_remove_test<element2>);
  run_test<element1>(edge_remove_test<element1>);
}
CATCH_TEST_CASE("Transpose Test", "[ds][adjmatrix]") {
  run_test<element8>(transpose_test<element8>);
  run_test<element4>(transpose_test<element4>);
  run_test<element2>(transpose_test<element2>);
  run_test<element1>(transpose_test<element1>);
}

CATCH_TEST_CASE("Print Test", "[ds][adjmatrix]") {
  run_test<element8>(print_test<element8>);
  run_test<element4>(print_test<element4>);
  run_test<element2>(print_test<element2>);
  run_test<element1>(print_test<element1>);
}
