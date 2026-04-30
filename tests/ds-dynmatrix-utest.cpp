/**
 * \file dynmatrix-test.cpp
 *
 * Test of dynamic matrix.
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

#include "rcsw/ds/dynmatrix.h"
#include "tests/ds_test.h"
#include "tests/ds_test.hpp"

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/
template <typename T>
void run_test(void (*test)(struct dynmatrix_config* config)) {
  struct dynmatrix_config config;
  memset(&config, 0, sizeof(dynmatrix_config));
  config.elt_size = sizeof(T);
  config.n_cols   = TH_NUM_ITEMS;
  config.n_rows   = TH_NUM_ITEMS;
  config.printe   = th::printe<T>;
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

      for (size_t m = 1; m <= 10; ++m) {
        for (size_t n = 1; n <= 10; ++n) {
          config.n_cols = m;
          config.n_rows = n;
          config.flags  = applied;
          test(&config);
        } /* for(m..) */
      } /* for(n..) */

      applied &= ~flags[j];
    } /* for(j..) */
  } /* for(i..) */

  th::ds_shutdown(&config);
} /* run_test() */

/*******************************************************************************
 * Test Functions
 ******************************************************************************/
template <typename T>
static void addremove_test(struct dynmatrix_config* config) {
  struct dynmatrix* matrix;
  struct dynmatrix  mymatrix;

  matrix = dynmatrix_init(&mymatrix, config);

  CATCH_REQUIRE(nullptr != matrix);
  th::element_generator<T> g(gen_elt_type::ekRAND_VALS,
                             config->n_rows * config->n_cols);

  for (size_t i = 0; i < config->n_rows; ++i) {
    for (size_t j = 0; j < config->n_cols; ++j) {
      T val = g.next();
      CATCH_REQUIRE(OK == dynmatrix_set(matrix, i, j, &val));
      CATCH_REQUIRE(0 ==
                    memcmp(&val, dynmatrix_access(matrix, i, j), sizeof(T)));
      CATCH_REQUIRE(OK == dynmatrix_clear(matrix, i, j));
      CATCH_REQUIRE(true ==
                    utils_zchk(dynmatrix_access(matrix, i, j), sizeof(T)));
    } /* for(j..) */
  } /* for(..) */

  dynmatrix_destroy(matrix);
}

template <typename T>
static void transpose_test(struct dynmatrix_config* config) {
  struct dynmatrix* matrix;
  struct dynmatrix  mymatrix;

  matrix = dynmatrix_init(&mymatrix, config);
  CATCH_REQUIRE(nullptr != matrix);

  /*
   * If the # of rows and columns isn't equal, the transpose will fail, so
   * don't try
   */
  if (config->n_rows != config->n_cols) {
    dynmatrix_destroy(matrix);
    return;
  }

  th::element_generator<T> g(gen_elt_type::ekRAND_VALS,
                             config->n_rows * config->n_cols);

  for (size_t i = 0; i < config->n_rows; ++i) {
    for (size_t j = 0; j < config->n_cols; ++j) {
      T val = g.next();
      CATCH_REQUIRE(OK == dynmatrix_set(matrix, i, j, &val));
      CATCH_REQUIRE(0 ==
                    memcmp(&val, dynmatrix_access(matrix, i, j), sizeof(T)));
      CATCH_REQUIRE(OK == dynmatrix_clear(matrix, i, j));
      CATCH_REQUIRE(utils_zchk(dynmatrix_access(matrix, i, j), sizeof(T)));
    } /* for(j..) */
  } /* for(..) */

  CATCH_REQUIRE(OK == dynmatrix_transpose(matrix));

  for (size_t i = 0; i < config->n_rows; ++i) {
    for (size_t j = 0; j < config->n_cols; ++j) {
      T* e1 = (T*)dynmatrix_access(matrix, i, j);
      T* e2 = (T*)dynmatrix_access(matrix, j, i);

      CATCH_REQUIRE(e1->value1 == e2->value1);
      if constexpr (!std::is_same<T, element1>::value) {
        CATCH_REQUIRE(e1->value2 == e2->value2);
      }
    } /* for(j..) */
  } /* for(..) */

  dynmatrix_destroy(matrix);
}

template <typename T>
static void print_test(struct dynmatrix_config* config) {
  struct dynmatrix* matrix;
  struct dynmatrix  mymatrix;

  dynmatrix_print(NULL);
  matrix = dynmatrix_init(&mymatrix, config);
  dynmatrix_print(matrix);

  CATCH_REQUIRE(nullptr != matrix);
  th::element_generator<T> g(gen_elt_type::ekRAND_VALS,
                             config->n_rows * config->n_cols);

  for (size_t i = 0; i < config->n_rows; ++i) {
    for (size_t j = 0; j < config->n_cols; ++j) {
      T val = g.next();
      CATCH_REQUIRE(OK == dynmatrix_set(matrix, i, j, &val));
      CATCH_REQUIRE(0 ==
                    memcmp(&val, dynmatrix_access(matrix, i, j), sizeof(T)));
      CATCH_REQUIRE(OK == dynmatrix_clear(matrix, i, j));
      CATCH_REQUIRE(true ==
                    utils_zchk(dynmatrix_access(matrix, i, j), sizeof(T)));
    } /* for(j..) */
  } /* for(..) */

  dynmatrix_print(matrix);
  dynmatrix_destroy(matrix);
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("Add/Remove Test", "[ds][dynmatrix]") {
  run_test<element8>(addremove_test<element8>);
  run_test<element4>(addremove_test<element4>);
  run_test<element2>(addremove_test<element2>);
  run_test<element1>(addremove_test<element1>);
}
CATCH_TEST_CASE("Transpose Test", "[ds][dynmatrix]") {
  run_test<element8>(transpose_test<element8>);
  run_test<element4>(transpose_test<element4>);
  run_test<element2>(transpose_test<element2>);
  run_test<element1>(transpose_test<element1>);
}
CATCH_TEST_CASE("Print Test", "[ds][dynmatrix]") {
  run_test<element8>(print_test<element8>);
  run_test<element4>(print_test<element4>);
  run_test<element2>(print_test<element2>);
  run_test<element1>(print_test<element1>);
}
