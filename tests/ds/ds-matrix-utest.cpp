/**
 * \file ds-matrix-utest.cpp
 *
 * Unit tests for matrix (static 2-D matrix backed by a flat array).
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

#include "rcsw/ds/matrix.h"
#include "tests/ds/ds_test.hpp"

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/
template <typename T>
static void run_test(void (*test)(struct matrix_config *)) {
  RCSW_ER_INIT(TH_ZLOG_CONF);

  struct matrix_config config;
  memset(&config, 0, sizeof(matrix_config));
  config.elt_size = sizeof(T);
  config.printe   = th::printe<T>;
  /* ds_init allocates the elements buffer at n_rows * n_cols * elt_size; set
   * the maximum dimensions used by the loops below so the buffer is large
   * enough for every (r, c) combination. */
  config.n_rows   = TH_NUM_ITEMS;
  config.n_cols   = TH_NUM_ITEMS;
  CATCH_REQUIRE(th::ds_init(&config) == OK);

  uint32_t flags[] = {
    RCSW_NONE,
    RCSW_ZALLOC,
    RCSW_NOALLOC_HANDLE,
    RCSW_NOALLOC_DATA,
  };

  /* Each flag in isolation */
  for (size_t i = 0; i < RCSW_ARRAY_ELTS(flags); ++i) {
    for (size_t r = 1; r <= (size_t)TH_NUM_ITEMS; ++r) {
      for (size_t c = 1; c <= (size_t)TH_NUM_ITEMS; ++c) {
        config.n_rows = r;
        config.n_cols = c;
        config.flags  = flags[i];
        test(&config);
      }
    }
  }

  /* Pairwise combinations */
  for (size_t i = 0; i < RCSW_ARRAY_ELTS(flags); ++i) {
    for (size_t j = i + 1; j < RCSW_ARRAY_ELTS(flags); ++j) {
      uint32_t applied = flags[i] | flags[j];
      for (size_t r = 1; r <= (size_t)TH_NUM_ITEMS; ++r) {
        for (size_t c = 1; c <= (size_t)TH_NUM_ITEMS; ++c) {
          config.n_rows = r;
          config.n_cols = c;
          config.flags  = applied;
          test(&config);
        }
      }
    }
  }

  th::ds_shutdown(&config);
  RCSW_ER_DEINIT();
}

/*******************************************************************************
 * Test Functions
 ******************************************************************************/
template <typename T>
static void addremove_test(struct matrix_config *config) {
  struct matrix *matrix;
  struct matrix  mymatrix;

  if (config->flags & RCSW_NOALLOC_HANDLE) {
    CATCH_REQUIRE(nullptr == matrix_init(nullptr, config));
  }
  matrix = matrix_init(&mymatrix, config);
  CATCH_REQUIRE(nullptr != matrix);

  th::element_generator<T> g(th::gen_elt_type::ekRAND_VALS,
                              config->n_rows * config->n_cols);

  for (size_t i = 0; i < config->n_rows; ++i) {
    for (size_t j = 0; j < config->n_cols; ++j) {
      T val = g.next();

      CATCH_REQUIRE(matrix_set(matrix, i, j, &val) == OK);
      CATCH_REQUIRE(
        memcmp(&val, matrix_access(matrix, i, j), sizeof(T)) == 0);

      CATCH_REQUIRE(matrix_elt_clear(matrix, i, j) == OK);
      CATCH_REQUIRE(utils_zchk(matrix_access(matrix, i, j), sizeof(T)));
    }
  }

  matrix_destroy(matrix);
}

template <typename T>
static void transpose_test(struct matrix_config *config) {
  struct matrix *matrix;
  struct matrix  mymatrix;

  matrix = matrix_init(&mymatrix, config);
  CATCH_REQUIRE(nullptr != matrix);

  th::element_generator<T> g(th::gen_elt_type::ekRAND_VALS,
                              config->n_rows * config->n_cols);

  /* Snapshot the values before transposing */
  std::vector<T> snapshot(config->n_rows * config->n_cols);
  for (size_t i = 0; i < config->n_rows; ++i) {
    for (size_t j = 0; j < config->n_cols; ++j) {
      T val = g.next();
      CATCH_REQUIRE(matrix_set(matrix, i, j, &val) == OK);
      snapshot[i * config->n_cols + j] = val;
    }
  }

  if (!matrix_issquare(matrix)) {
    CATCH_REQUIRE(matrix_transpose(matrix) == ERROR);
  } else {
    CATCH_REQUIRE(matrix_transpose(matrix) == OK);

    /* After transpose: element [i][j] == original [j][i] */
    for (size_t i = 0; i < config->n_rows; ++i) {
      for (size_t j = 0; j < config->n_cols; ++j) {
        T *transposed = (T *)matrix_access(matrix, i, j);
        T &original   = snapshot[j * config->n_cols + i];

        CATCH_REQUIRE(transposed->value1 == original.value1);
        if constexpr (!std::is_same<T, element1>::value) {
          /* Original had e1->value2 == e2->value2; the old code compared
           * e1->value2 == e1->value2 (self-comparison, always true). */
          CATCH_REQUIRE(transposed->value2 == original.value2);
        }
      }
    }
  }

  matrix_destroy(matrix);
}

template <typename T>
static void print_test(struct matrix_config *config) {
  struct matrix *matrix;
  struct matrix  mymatrix;

  /* NULL handle must not crash */
  matrix_print(nullptr);

  matrix = matrix_init(&mymatrix, config);
  CATCH_REQUIRE(nullptr != matrix);

  matrix_print(matrix); /* empty */

  th::element_generator<T> g(th::gen_elt_type::ekRAND_VALS,
                              config->n_rows * config->n_cols);
  for (size_t i = 0; i < config->n_rows; ++i) {
    for (size_t j = 0; j < config->n_cols; ++j) {
      T val = g.next();
      CATCH_REQUIRE(matrix_set(matrix, i, j, &val) == OK);
    }
  }

  matrix_print(matrix); /* populated */
  matrix_destroy(matrix);
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("matrix Add/Remove Test", "[ds][matrix]") {
  run_test<element8>(addremove_test<element8>);
  run_test<element4>(addremove_test<element4>);
  run_test<element2>(addremove_test<element2>);
  run_test<element1>(addremove_test<element1>);
}

CATCH_TEST_CASE("matrix Transpose Test", "[ds][matrix]") {
  run_test<element8>(transpose_test<element8>);
  run_test<element4>(transpose_test<element4>);
  run_test<element2>(transpose_test<element2>);
  run_test<element1>(transpose_test<element1>);
}

CATCH_TEST_CASE("matrix Print Test", "[ds][matrix]") {
  run_test<element8>(print_test<element8>);
  run_test<element4>(print_test<element4>);
  run_test<element2>(print_test<element2>);
  run_test<element1>(print_test<element1>);
}
