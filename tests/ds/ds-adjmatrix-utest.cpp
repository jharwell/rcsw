/**
 * \file ds-adjmatrix-utest.cpp
 *
 * Unit tests for adjmatrix (adjacency matrix for graphs).
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
#include "tests/ds/ds_test.hpp"

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/
/*
 * The adjmatrix element type is always double (edge weights) or boolean
 * (unweighted); the T parameter is only used to vary elt_size for coverage.
 */
template <typename T>
static void run_test(void (*test)(struct adjmatrix_config *)) {
  struct adjmatrix_config config;
  memset(&config, 0, sizeof(adjmatrix_config));
  config.elt_size    = sizeof(T);
  config.n_vertices  = TH_NUM_ITEMS;
  CATCH_REQUIRE(th::ds_init(&config) == OK);

  uint32_t flags[] = {
    RCSW_NONE,
    RCSW_ZALLOC,
    RCSW_NOALLOC_HANDLE,
    RCSW_NOALLOC_DATA,
  };

  /* is_directed × is_weighted combinations:
   *   (directed=false, weighted=false) → undirected unweighted
   *   (directed=true,  weighted=false) → directed unweighted
   *   (directed=true,  weighted=true)  → directed weighted
   * (directed=false, weighted=true) is illegal and skipped. */
  struct { bool_t directed; bool_t weighted; } graph_types[] = {
    { false, false },
    { true,  false },
    { true,  true  },
  };

  /* Each allocation flag in isolation, all graph types */
  for (size_t i = 0; i < RCSW_ARRAY_ELTS(flags); ++i) {
    for (auto &gt : graph_types) {
      config.flags       = flags[i];
      config.is_directed = gt.directed;
      config.is_weighted = gt.weighted;
      test(&config);
    }
  }

  /* Pairwise allocation flag combinations, all graph types */
  for (size_t i = 0; i < RCSW_ARRAY_ELTS(flags); ++i) {
    for (size_t j = i + 1; j < RCSW_ARRAY_ELTS(flags); ++j) {
      uint32_t applied = flags[i] | flags[j];
      for (auto &gt : graph_types) {
        config.flags       = applied;
        config.is_directed = gt.directed;
        config.is_weighted = gt.weighted;
        test(&config);
      }
    }
  }

  th::ds_shutdown(&config);
}

/*******************************************************************************
 * Test Functions
 ******************************************************************************/
template <typename T>
static void edge_add_test(struct adjmatrix_config *config) {
  struct adjmatrix *matrix;
  struct adjmatrix  mymatrix;

  if (config->flags & RCSW_NOALLOC_HANDLE) {
    CATCH_REQUIRE(nullptr == adjmatrix_init(nullptr, config));
  }
  matrix = adjmatrix_init(&mymatrix, config);
  CATCH_REQUIRE(nullptr != matrix);

  /* Initially no edges */
  CATCH_REQUIRE(adjmatrix_isempty(matrix));

  for (size_t i = 1; i < config->n_vertices; ++i) {
    if (matrix->is_directed) {
      double val = rand() % 10 + 1;
      CATCH_REQUIRE(adjmatrix_edge_addd(matrix, i - 1, i, &val) == OK);
    } else {
      CATCH_REQUIRE(adjmatrix_edge_addu(matrix, i - 1, i) == OK);
    }

    CATCH_REQUIRE(adjmatrix_edge_query(matrix, i - 1, i) == true);

    /* Undirected: reverse direction must also be set */
    if (!matrix->is_directed) {
      CATCH_REQUIRE(adjmatrix_edge_query(matrix, i, i - 1) == true);
    }
  }

  adjmatrix_destroy(matrix);
}

template <typename T>
static void edge_remove_test(struct adjmatrix_config *config) {
  struct adjmatrix *matrix;
  struct adjmatrix  mymatrix;

  matrix = adjmatrix_init(&mymatrix, config);
  CATCH_REQUIRE(nullptr != matrix);

  /* Build a chain of edges */
  for (size_t i = 1; i < config->n_vertices; ++i) {
    if (matrix->is_directed) {
      double val = rand() % 10 + 1;
      CATCH_REQUIRE(adjmatrix_edge_addd(matrix, i - 1, i, &val) == OK);
    } else {
      CATCH_REQUIRE(adjmatrix_edge_addu(matrix, i - 1, i) == OK);
    }
  }

  /* Remove edges randomly until the matrix is empty */
  size_t max = config->n_vertices;
  while (!adjmatrix_isempty(matrix)) {
    size_t u = rand() % max;
    size_t v = rand() % max;
    if (adjmatrix_edge_query(matrix, u, v)) {
      CATCH_REQUIRE(adjmatrix_edge_remove(matrix, u, v) == OK);
      CATCH_REQUIRE(adjmatrix_edge_query(matrix, u, v) == false);
      if (!matrix->is_directed) {
        CATCH_REQUIRE(adjmatrix_edge_query(matrix, v, u) == false);
      }
    }
  }

  adjmatrix_destroy(matrix);
}

template <typename T>
static void transpose_test(struct adjmatrix_config *config) {
  struct adjmatrix *matrix;
  struct adjmatrix  mymatrix;

  matrix = adjmatrix_init(&mymatrix, config);
  CATCH_REQUIRE(nullptr != matrix);

  /* Build a chain */
  for (size_t i = 1; i < config->n_vertices; ++i) {
    if (matrix->is_directed) {
      double val = rand() % 10 + 1;
      CATCH_REQUIRE(adjmatrix_edge_addd(matrix, i - 1, i, &val) == OK);
    } else {
      CATCH_REQUIRE(adjmatrix_edge_addu(matrix, i - 1, i) == OK);
    }
  }

  CATCH_REQUIRE(adjmatrix_transpose(matrix) == OK);

  /* After transpose each directed edge i-1 → i should appear as i → i-1 */
  for (size_t i = 1; i < config->n_vertices; ++i) {
    CATCH_REQUIRE(adjmatrix_edge_query(matrix, i, i - 1) == true);
  }

  adjmatrix_destroy(matrix);
}

template <typename T>
static void print_test(struct adjmatrix_config *config) {
  struct adjmatrix *matrix;
  struct adjmatrix  mymatrix;

  /* NULL handle must not crash */
  adjmatrix_print(nullptr);

  matrix = adjmatrix_init(&mymatrix, config);
  CATCH_REQUIRE(nullptr != matrix);

  adjmatrix_print(matrix); /* empty */

  for (size_t i = 1; i < config->n_vertices; ++i) {
    if (matrix->is_directed) {
      double val = rand() % 10 + 1;
      CATCH_REQUIRE(adjmatrix_edge_addd(matrix, i - 1, i, &val) == OK);
    } else {
      CATCH_REQUIRE(adjmatrix_edge_addu(matrix, i - 1, i) == OK);
    }
  }

  adjmatrix_print(matrix); /* populated */
  adjmatrix_destroy(matrix);
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("adjmatrix Edge Add Test", "[ds][adjmatrix]") {
  run_test<element8>(edge_add_test<element8>);
  run_test<element4>(edge_add_test<element4>);
  run_test<element2>(edge_add_test<element2>);
  run_test<element1>(edge_add_test<element1>);
}

CATCH_TEST_CASE("adjmatrix Edge Remove Test", "[ds][adjmatrix]") {
  run_test<element8>(edge_remove_test<element8>);
  run_test<element4>(edge_remove_test<element4>);
  run_test<element2>(edge_remove_test<element2>);
  run_test<element1>(edge_remove_test<element1>);
}

CATCH_TEST_CASE("adjmatrix Transpose Test", "[ds][adjmatrix]") {
  run_test<element8>(transpose_test<element8>);
  run_test<element4>(transpose_test<element4>);
  run_test<element2>(transpose_test<element2>);
  run_test<element1>(transpose_test<element1>);
}

CATCH_TEST_CASE("adjmatrix Print Test", "[ds][adjmatrix]") {
  run_test<element8>(print_test<element8>);
  run_test<element4>(print_test<element4>);
  run_test<element2>(print_test<element2>);
  run_test<element1>(print_test<element1>);
}
