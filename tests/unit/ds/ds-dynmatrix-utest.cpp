/**
 * \file
 *
 * \copyright 2017 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * Unit tests for dynmatrix (heap-allocated 2-D matrix).
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <limits.h>
#define CATCH_CONFIG_PREFIX_ALL
#include <catch2/catch_test_macros.hpp>

#include "rcsw/ds/dynmatrix.h"
#include "tests/unit/ds/ds_test.hpp"

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/
template <typename T>
static void run_test(void (*test)(struct dynmatrix_config*)) {
  struct dynmatrix_config config;
  memset(&config, 0, sizeof(dynmatrix_config));
  config.elt_size = sizeof(T);
  config.printe   = th::printe<T>;
  CATCH_REQUIRE(th::ds_init(&config) == OK);

  uint32_t flags[] = {
    RCSW_NONE,
    RCSW_ZALLOC,
    RCSW_NOALLOC_HANDLE,
    RCSW_NOALLOC_DATA,
  };

  /* Each flag in isolation */
  for (size_t i = 0; i < RCSW_ARRAY_ELTS(flags); ++i) {
    for (size_t r = 1; r <= 10; ++r) {
      for (size_t c = 1; c <= 10; ++c) {
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
      for (size_t r = 1; r <= 10; ++r) {
        for (size_t c = 1; c <= 10; ++c) {
          config.n_rows = r;
          config.n_cols = c;
          config.flags  = applied;
          test(&config);
        }
      }
    }
  }

  th::ds_shutdown(&config);
}

struct dynmatrix* make_dyn(size_t rows, size_t cols) {
  struct dynmatrix_config c;
  memset(&c, 0, sizeof(c));
  c.elt_size = sizeof(int);
  c.n_rows   = rows;
  c.n_cols   = cols;
  return dynmatrix_init(nullptr, &c);
}

/*******************************************************************************
 * Test Functions
 ******************************************************************************/
template <typename T>
static void addremove_test(struct dynmatrix_config* config) {
  struct dynmatrix* matrix;
  struct dynmatrix  mymatrix;

  if (config->flags & RCSW_NOALLOC_HANDLE) {
    CATCH_REQUIRE(nullptr == dynmatrix_init(nullptr, config));
  }
  matrix = dynmatrix_init(&mymatrix, config);
  CATCH_REQUIRE(nullptr != matrix);

  th::element_generator<T> g(th::gen_elt_type::RAND_VALS,
                             config->n_rows * config->n_cols);

  for (size_t i = 0; i < config->n_rows; ++i) {
    for (size_t j = 0; j < config->n_cols; ++j) {
      T val = g.next();

      CATCH_REQUIRE(dynmatrix_set(matrix, i, j, &val) == OK);
      CATCH_REQUIRE(memcmp(&val, dynmatrix_access(matrix, i, j), sizeof(T)) == 0);

      CATCH_REQUIRE(dynmatrix_clear(matrix, i, j) == OK);
      CATCH_REQUIRE(utils_zchk(dynmatrix_access(matrix, i, j), sizeof(T)) ==
                    true);
    }
  }

  dynmatrix_destroy(matrix);
}

template <typename T>
static void transpose_test(struct dynmatrix_config* config) {
  struct dynmatrix* matrix;
  struct dynmatrix  mymatrix;

  matrix = dynmatrix_init(&mymatrix, config);
  CATCH_REQUIRE(nullptr != matrix);

  /*
   * dynmatrix_transpose() requires a square matrix; verify the contract and
   * skip the ordering check for non-square shapes.
   */
  if (config->n_rows != config->n_cols) {
    CATCH_REQUIRE(dynmatrix_transpose(matrix) == ERROR);
    dynmatrix_destroy(matrix);
    return;
  }

  th::element_generator<T> g(th::gen_elt_type::RAND_VALS,
                             config->n_rows * config->n_cols);

  /* Snapshot values before transposing */
  std::vector<T> snapshot(config->n_rows * config->n_cols);
  for (size_t i = 0; i < config->n_rows; ++i) {
    for (size_t j = 0; j < config->n_cols; ++j) {
      T val = g.next();
      CATCH_REQUIRE(dynmatrix_set(matrix, i, j, &val) == OK);
      snapshot[(i * config->n_cols) + j] = val;
    }
  }

  CATCH_REQUIRE(dynmatrix_transpose(matrix) == OK);

  /* After transpose: element [i][j] should equal original [j][i] */
  for (size_t i = 0; i < config->n_rows; ++i) {
    for (size_t j = 0; j < config->n_cols; ++j) {
      T* transposed = (T*)dynmatrix_access(matrix, i, j);
      T& original   = snapshot[(j * config->n_cols) + i];
      CATCH_REQUIRE(transposed->value1 == original.value1);
      if constexpr (!std::is_same<T, element1>::value) {
        CATCH_REQUIRE(transposed->value2 == original.value2);
      }
    }
  }

  dynmatrix_destroy(matrix);
}

template <typename T>
static void print_test(struct dynmatrix_config* config) {
  struct dynmatrix* matrix;
  struct dynmatrix  mymatrix;

  /* NULL handle must not crash */
  dynmatrix_print(nullptr);

  matrix = dynmatrix_init(&mymatrix, config);
  CATCH_REQUIRE(nullptr != matrix);

  dynmatrix_print(matrix); /* empty */

  th::element_generator<T> g(th::gen_elt_type::RAND_VALS,
                             config->n_rows * config->n_cols);
  for (size_t i = 0; i < config->n_rows; ++i) {
    for (size_t j = 0; j < config->n_cols; ++j) {
      T val = g.next();
      CATCH_REQUIRE(dynmatrix_set(matrix, i, j, &val) == OK);
    }
  }

  dynmatrix_print(matrix); /* populated */
  dynmatrix_destroy(matrix);
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("dynmatrix Add/Remove Test", "[ds][dynmatrix]") {
  run_test<element8>(addremove_test<element8>);
  run_test<element4>(addremove_test<element4>);
  run_test<element2>(addremove_test<element2>);
  run_test<element1>(addremove_test<element1>);
}

CATCH_TEST_CASE("dynmatrix Transpose Test", "[ds][dynmatrix]") {
  run_test<element8>(transpose_test<element8>);
  run_test<element4>(transpose_test<element4>);
  run_test<element2>(transpose_test<element2>);
  run_test<element1>(transpose_test<element1>);
}

CATCH_TEST_CASE("dynmatrix Print Test", "[ds][dynmatrix]") {
  run_test<element8>(print_test<element8>);
  run_test<element4>(print_test<element4>);
  run_test<element2>(print_test<element2>);
  run_test<element1>(print_test<element1>);
}

CATCH_TEST_CASE("dynmatrix_set beyond the current dimensions grows the matrix",
                "[ds][dynmatrix]") {
  struct dynmatrix* m = make_dyn(2, 2);
  CATCH_REQUIRE(nullptr != m);
  int a = 1;
  int b = 7;
  CATCH_REQUIRE(OK == dynmatrix_set(m, 0, 0, &a));
  CATCH_REQUIRE(OK == dynmatrix_set(m, 3, 3, &b));
  CATCH_REQUIRE(4 == m->n_rows);
  CATCH_REQUIRE(4 == m->n_cols);
  CATCH_REQUIRE(1 == *static_cast<int*>(dynmatrix_access(m, 0, 0)));
  CATCH_REQUIRE(7 == *static_cast<int*>(dynmatrix_access(m, 3, 3)));
  CATCH_REQUIRE(0 == *static_cast<int*>(dynmatrix_access(m, 3, 0)));
  dynmatrix_destroy(m);
}

CATCH_TEST_CASE("dynmatrix_resize can shrink the matrix", "[ds][dynmatrix]") {
  struct dynmatrix* m = make_dyn(4, 4);
  CATCH_REQUIRE(nullptr != m);
  CATCH_REQUIRE(OK == dynmatrix_resize(m, 2, 3));
  CATCH_REQUIRE(2 == m->n_rows);
  CATCH_REQUIRE(3 == m->n_cols);
  dynmatrix_destroy(m);
}
