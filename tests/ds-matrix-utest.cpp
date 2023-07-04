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
static void test_runner(void (*test)(struct ds_params *params)) {
  /* dbg_init(); */
  /* dbg_insmod(M_TESTING,"Testing"); */
  /* dbg_insmod(M_DS_MATRIX,"MATRIX"); */
  /* dbg_mod_lvl_set(M_DS_MATRIX, DBG_V); */

  struct ds_params params;
  params.tag = ekRCSW_DS_MATRIX;
  params.elt_size = sizeof(T);
  params.type.smat.n_cols = TH_NUM_ITEMS;
  params.type.smat.n_rows = TH_NUM_ITEMS;
  params.printe = th_printe<T>;
  CATCH_REQUIRE(th_ds_init(&params) == OK);

  uint32_t flags[] = {
    RCSW_DS_NOALLOC_HANDLE,
    RCSW_DS_NOALLOC_DATA,
  };
  for (size_t i = 1; i <= 10; ++i) {
    for (size_t j = 1; j <= 10; ++j) {
      for (size_t k = 0; k < RCSW_ARRAY_SIZE(flags); ++k) {
      params.type.smat.n_cols = i;
      params.type.smat.n_rows = j;
      params.flags = flags[k];
      test(&params);
      } /* for(k..) */
    } /* for(j..) */
  } /* for(i..) */
  th_ds_shutdown(&params);
} /* test_runner() */

/*******************************************************************************
 * Test Functions
 ******************************************************************************/
template<typename T>
static void addremove_test(struct ds_params* params) {
  struct matrix *matrix;
  struct matrix mymatrix;

  if (params->flags & RCSW_DS_NOALLOC_HANDLE) {
    CATCH_REQUIRE(nullptr == matrix_init(NULL, params));
  }
  matrix = matrix_init(&mymatrix, params);
  CATCH_REQUIRE(nullptr != matrix);


  CATCH_REQUIRE(nullptr != matrix);
  element_generator<T> g(gen_elt_type::ekRAND_VALS,
                         params->type.smat.n_rows * params->type.smat.n_cols);
  for (size_t i = 0; i < params->type.smat.n_rows; ++i) {
    for (size_t j = 0; j < params->type.smat.n_cols; ++j) {
      T val = g.next();
      CATCH_REQUIRE(OK == matrix_set(matrix, i, j, &val));
      CATCH_REQUIRE(0 == memcmp(&val,
                                matrix_access(matrix, i, j),
                                sizeof(T)));
      CATCH_REQUIRE(OK == matrix_elt_clear(matrix, i, j));
      CATCH_REQUIRE(ds_elt_zchk(matrix_access(matrix, i, j),
                                        sizeof(T)));
    } /* for(j..) */
  } /* for(..) */

  matrix_destroy(matrix);
}

template<typename T>
static void transpose_test(struct ds_params* params) {
  struct matrix *matrix;
  struct matrix mymatrix;

  matrix = matrix_init(&mymatrix, params);
  CATCH_REQUIRE(nullptr != matrix);

  element_generator<T> g(gen_elt_type::ekRAND_VALS,
                         params->type.smat.n_rows * params->type.smat.n_cols);

  for (size_t i = 0; i < params->type.smat.n_rows; ++i) {
    for (size_t j = 0; j < params->type.smat.n_cols; ++j) {
      T val = g.next();
      CATCH_REQUIRE(OK == matrix_set(matrix, i, j, &val));
      CATCH_REQUIRE(0 == memcmp(&val, matrix_access(matrix, i, j),
                                sizeof(T)));
      CATCH_REQUIRE(OK == matrix_elt_clear(matrix, i, j));
      CATCH_REQUIRE(ds_elt_zchk(matrix_access(matrix, i, j),
                                sizeof(T)));
    } /* for(j..) */
  } /* for(..) */

  if (!matrix_issquare(matrix)) {
    CATCH_REQUIRE(ERROR == matrix_transpose(matrix));
  } else {
    CATCH_REQUIRE(OK == matrix_transpose(matrix));

    for (size_t i = 0; i < params->type.smat.n_rows; ++i) {
      for (size_t j = 0; j < params->type.smat.n_cols; ++j) {
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
static void print_test(struct ds_params* params) {
  struct matrix *matrix;
  struct matrix mymatrix;

  matrix = matrix_init(&mymatrix, params);
  CATCH_REQUIRE(nullptr != matrix);
  matrix_print(matrix);

  element_generator<T> g(gen_elt_type::ekRAND_VALS,
                         params->type.smat.n_rows * params->type.smat.n_cols);

  for (size_t i = 0; i < params->type.smat.n_rows; ++i) {
    for (size_t j = 0; j < params->type.smat.n_cols; ++j) {
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
