/**
 * \file matrix-test.cpp
 *
 * Test of static matrix.
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

#include "rcsw/ds/matrix.h"
#include "tests/ds_test.h"
#include "tests/ds_test.hpp"

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/
template<typename T>
static void test_runner(void (*test)(struct matrix_params *params)) {
  struct matrix_params params;
  memset(&params, 0, sizeof(matrix_params));
  params.elt_size = sizeof(T);
  params.n_cols = TH_NUM_ITEMS;
  params.n_rows = TH_NUM_ITEMS;
  params.printe = th::printe<T>;
  CATCH_REQUIRE(th::ds_init(&params) == OK);

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

      for (size_t k = 1; k <= TH_NUM_ITEMS; ++k) {
        for (size_t m = 1; m <= TH_NUM_ITEMS; ++m) {
          params.n_cols = k;
          params.n_rows = m;
          params.flags = applied;
          test(&params);
        } /* for(m..) */
      } /* for(k..) */


      applied &= ~flags[j];
    } /* for(j..) */
  } /* for(i..) */

  th::ds_shutdown(&params);
} /* test_runner() */

/*******************************************************************************
 * Test Functions
 ******************************************************************************/
template<typename T>
static void addremove_test(struct matrix_params* params) {
  struct matrix *matrix;
  struct matrix mymatrix;

  if (params->flags & RCSW_NOALLOC_HANDLE) {
    CATCH_REQUIRE(nullptr == matrix_init(NULL, params));
  }
  matrix = matrix_init(&mymatrix, params);
  CATCH_REQUIRE(nullptr != matrix);


  CATCH_REQUIRE(nullptr != matrix);
  th::element_generator<T> g(gen_elt_type::ekRAND_VALS,
                         params->n_rows * params->n_cols);
  for (size_t i = 0; i < params->n_rows; ++i) {
    for (size_t j = 0; j < params->n_cols; ++j) {
      T val = g.next();
      CATCH_REQUIRE(OK == matrix_set(matrix, i, j, &val));
      CATCH_REQUIRE(0 == memcmp(&val,
                                matrix_access(matrix, i, j),
                                sizeof(T)));
      CATCH_REQUIRE(OK == matrix_elt_clear(matrix, i, j));
      CATCH_REQUIRE(util_zchk(matrix_access(matrix, i, j),
                                        sizeof(T)));
    } /* for(j..) */
  } /* for(..) */

  matrix_destroy(matrix);
}

template<typename T>
static void transpose_test(struct matrix_params* params) {
  struct matrix *matrix;
  struct matrix mymatrix;

  matrix = matrix_init(&mymatrix, params);
  CATCH_REQUIRE(nullptr != matrix);

  th::element_generator<T> g(gen_elt_type::ekRAND_VALS,
                         params->n_rows * params->n_cols);

  for (size_t i = 0; i < params->n_rows; ++i) {
    for (size_t j = 0; j < params->n_cols; ++j) {
      T val = g.next();
      CATCH_REQUIRE(OK == matrix_set(matrix, i, j, &val));
      CATCH_REQUIRE(0 == memcmp(&val, matrix_access(matrix, i, j),
                                sizeof(T)));
      CATCH_REQUIRE(OK == matrix_elt_clear(matrix, i, j));
      CATCH_REQUIRE(util_zchk(matrix_access(matrix, i, j),
                                sizeof(T)));
    } /* for(j..) */
  } /* for(..) */

  if (!matrix_issquare(matrix)) {
    CATCH_REQUIRE(ERROR == matrix_transpose(matrix));
  } else {
    CATCH_REQUIRE(OK == matrix_transpose(matrix));

    for (size_t i = 0; i < params->n_rows; ++i) {
      for (size_t j = 0; j < params->n_cols; ++j) {
        T* e1 = (T*)matrix_access(matrix, i, j);
        T* e2 = (T*)matrix_access(matrix, j, i);

        CATCH_REQUIRE(e1->value1 == e2->value1);
        if constexpr (!std::is_same<T, element1>::value) {
            CATCH_REQUIRE(e1->value2 == e1->value2);
        }
      } /* for(j..) */
    } /* for(..) */
  }
  matrix_destroy(matrix);
}

template<typename T>
static void print_test(struct matrix_params* params) {
  struct matrix *matrix;
  struct matrix mymatrix;

  matrix = matrix_init(&mymatrix, params);
  CATCH_REQUIRE(nullptr != matrix);

  th::element_generator<T> g(gen_elt_type::ekRAND_VALS,
                         params->n_rows * params->n_cols);

  for (size_t i = 0; i < params->n_rows; ++i) {
    for (size_t j = 0; j < params->n_cols; ++j) {
      T val = g.next();
      CATCH_REQUIRE(OK == matrix_set(matrix, i, j, &val));
    } /* for(j..) */
  } /* for(..) */

  matrix_print(matrix);
  matrix_destroy(matrix);
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("Add/Remove Test", "[ds][matrix]") {
  test_runner<element8>(addremove_test<element8>);
  test_runner<element4>(addremove_test<element4>);
  test_runner<element2>(addremove_test<element2>);
  test_runner<element1>(addremove_test<element1>);
}
CATCH_TEST_CASE("Transpose Test", "[ds][matrix]") {
  test_runner<element8>(transpose_test<element8>);
  test_runner<element4>(transpose_test<element4>);
  test_runner<element2>(transpose_test<element2>);
  test_runner<element1>(transpose_test<element1>);
}
CATCH_TEST_CASE("Print Test", "[ds][matrix]") {
  test_runner<element8>(print_test<element8>);
  test_runner<element4>(print_test<element4>);
  test_runner<element2>(print_test<element2>);
  test_runner<element1>(print_test<element1>);
}
