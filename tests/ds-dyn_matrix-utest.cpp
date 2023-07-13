/**
 * \file dyn_matrix-test.cpp
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

#include "rcsw/ds/dyn_matrix.h"
#include "tests/ds_test.h"
#include "tests/ds_test.hpp"

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/
template<typename T>
void run_test(void (*test)(struct dyn_matrix_params *params)) {
  /* dbg_init(); */
  /* dbg_insmod(M_TESTING,"Testing"); */
  /* dbg_insmod(M_DS_DYN_MATRIX,"DYN_MATRIX"); */
  /* dbg_mod_lvl_set(M_DS_DYN_MATRIX, DBG_V); */

  struct dyn_matrix_params params;
  params.elt_size = sizeof(T);
  params.n_cols = TH_NUM_ITEMS;
  params.n_rows = TH_NUM_ITEMS;
  params.printe = th_printe<T>;
  CATCH_REQUIRE(th_ds_init(&params) == OK);

    uint32_t flags[] = {
    RCSW_NOALLOC_HANDLE,
    RCSW_NOALLOC_DATA,
  };
  for (size_t i = 1; i <= 10; ++i) {
    for (size_t j = 1; j <= 10; ++j) {
      for (size_t k = 0; k < RCSW_ARRAY_ELTS(flags); ++k) {
      params.n_cols = i;
      params.n_rows = j;
      params.flags = flags[k];
      test(&params);
      } /* for(k..) */
    } /* for(j..) */
  } /* for(i..) */
  th_ds_shutdown(&params);
} /* run_test() */

/*******************************************************************************
 * Test Functions
 ******************************************************************************/
template<typename T>
static void addremove_test(struct dyn_matrix_params* params) {
  struct dyn_matrix *matrix;
  struct dyn_matrix mymatrix;

  matrix = dyn_matrix_init(&mymatrix, params);

  CATCH_REQUIRE(nullptr != matrix);
  element_generator<T> g(gen_elt_type::ekRAND_VALS,
                         params->n_rows * params->n_cols);

  for (size_t i = 0; i < params->n_rows; ++i) {
    for (size_t j = 0; j < params->n_cols; ++j) {
      T val = g.next();
      CATCH_REQUIRE(OK == dyn_matrix_set(matrix, i, j, &val));
      CATCH_REQUIRE(0 == memcmp(&val, dyn_matrix_access(matrix, i, j),
                                sizeof(T)));
      CATCH_REQUIRE(OK == dyn_matrix_clear(matrix, i, j));
      CATCH_REQUIRE(true == util_zchk(dyn_matrix_access(matrix, i, j),
                                        sizeof(T)));
    } /* for(j..) */
  } /* for(..) */

  dyn_matrix_destroy(matrix);
}

template<typename T>
static void transpose_test(struct dyn_matrix_params* params) {
  struct dyn_matrix *matrix;
  struct dyn_matrix mymatrix;

  matrix = dyn_matrix_init(&mymatrix, params);
  CATCH_REQUIRE(nullptr != matrix);

  /*
   * If the # of rows and columns isn't equal, the transpose will fail, so
   * don't try
   */
  if (params->n_rows != params->n_cols) {
    dyn_matrix_destroy(matrix);
    return;
  }

  element_generator<T> g(gen_elt_type::ekRAND_VALS,
                         params->n_rows * params->n_cols);

  for (size_t i = 0; i < params->n_rows; ++i) {
    for (size_t j = 0; j < params->n_cols; ++j) {
      T val = g.next();
      CATCH_REQUIRE(OK == dyn_matrix_set(matrix, i, j, &val));
      CATCH_REQUIRE(0 == memcmp(&val, dyn_matrix_access(matrix, i, j),
                                sizeof(T)));
      CATCH_REQUIRE(OK == dyn_matrix_clear(matrix, i, j));
      CATCH_REQUIRE(util_zchk(dyn_matrix_access(matrix, i, j),
                                sizeof(T)));
    } /* for(j..) */
  } /* for(..) */

  CATCH_REQUIRE(OK == dyn_matrix_transpose(matrix));

  for (size_t i = 0; i < params->n_rows; ++i) {
    for (size_t j = 0; j < params->n_cols; ++j) {
      T* e1 = (T*)dyn_matrix_access(matrix, i, j);
      T* e2 = (T*)dyn_matrix_access(matrix, j, i);

      CATCH_REQUIRE(e1->value1 == e2->value1);
      if constexpr (!std::is_same<T, element1>::value) {
          CATCH_REQUIRE(e1->value2 == e2->value2);
      }
    } /* for(j..) */
  } /* for(..) */

  dyn_matrix_destroy(matrix);
}

template<typename T>
static void print_test(struct dyn_matrix_params* params) {
  struct dyn_matrix *matrix;
  struct dyn_matrix mymatrix;

  dyn_matrix_print(NULL);
  matrix = dyn_matrix_init(&mymatrix, params);
  dyn_matrix_print(matrix);

  CATCH_REQUIRE(nullptr != matrix);
  element_generator<T> g(gen_elt_type::ekRAND_VALS,
                         params->n_rows * params->n_cols);

  for (size_t i = 0; i < params->n_rows; ++i) {
    for (size_t j = 0; j < params->n_cols; ++j) {
      T val = g.next();
      CATCH_REQUIRE(OK == dyn_matrix_set(matrix, i, j, &val));
      CATCH_REQUIRE(0 == memcmp(&val, dyn_matrix_access(matrix, i, j),
                                sizeof(T)));
      CATCH_REQUIRE(OK == dyn_matrix_clear(matrix, i, j));
      CATCH_REQUIRE(true == util_zchk(dyn_matrix_access(matrix, i, j),
                                        sizeof(T)));
    } /* for(j..) */
  } /* for(..) */

  dyn_matrix_print(matrix);
  dyn_matrix_destroy(matrix);
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("Add/Remove Test", "[ds][dyn_matrix]") {
  run_test<element8>(addremove_test<element8>);
  run_test<element4>(addremove_test<element4>);
  run_test<element2>(addremove_test<element2>);
  run_test<element1>(addremove_test<element1>);
}
CATCH_TEST_CASE("Transpose Test", "[ds][dyn_matrix]") {
  run_test<element8>(transpose_test<element8>);
  run_test<element4>(transpose_test<element4>);
  run_test<element2>(transpose_test<element2>);
  run_test<element1>(transpose_test<element1>);
}
CATCH_TEST_CASE("Print Test", "[ds][dyn_matrix]") {
  run_test<element8>(print_test<element8>);
  run_test<element4>(print_test<element4>);
  run_test<element2>(print_test<element2>);
  run_test<element1>(print_test<element1>);
}
