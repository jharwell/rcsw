/**
 * \file static_matrix-test.cpp
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

#include "rcsw/ds/static_matrix.h"
#include "tests/ds_test.h"

/*******************************************************************************
 * Test Functions
 ******************************************************************************/
/* test functions */
static void addremove_test(struct ds_params* params);
static void transpose_test(struct ds_params* params);

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/
static void test_runner(void (*test)(struct ds_params *params)) {
  /* dbg_init(); */
  /* dbg_insmod(M_TESTING,"Testing"); */
  /* dbg_insmod(M_DS_STATIC_MATRIX,"STATIC_MATRIX"); */
  /* dbg_mod_lvl_set(M_DS_STATIC_MATRIX, DBG_V); */

  struct ds_params params;
  params.tag = ekRCSW_DS_STATIC_MATRIX;
  params.elt_size = sizeof(struct element8);
  params.type.smat.n_cols = TH_NUM_ITEMS;
  params.type.smat.n_rows = TH_NUM_ITEMS;
  CATCH_REQUIRE(th_ds_init(&params) == OK);

  for (size_t i = 1; i <= 10; ++i) {
    for (size_t j = 1; j <= 10; ++j) {
      params.type.smat.n_cols = i;
      params.type.smat.n_rows = j;
      params.flags = 0;
      test(&params);
      params.flags = RCSW_DS_NOALLOC_DATA;
      test(&params);
    } /* for(j..) */
  } /* for(i..) */
  th_ds_shutdown(&params);
} /* test_runner() */

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("Add/Remove Test", "[static_matrix]") { test_runner(addremove_test); }
CATCH_TEST_CASE("Transpose Test", "[static_matrix]") { test_runner(transpose_test); }

static void addremove_test(struct ds_params* params) {
  struct static_matrix *matrix;
  struct static_matrix mymatrix;

  matrix = static_matrix_init(&mymatrix, params);

  CATCH_REQUIRE(NULL != matrix);
  for (size_t i = 0; i < params->type.smat.n_rows; ++i) {
    for (size_t j = 0; j < params->type.smat.n_cols; ++j) {
      struct element8 val = { rand(), rand() };
  CATCH_REQUIRE(OK == static_matrix_set(matrix, i, j, &val));
      CATCH_REQUIRE(0 == memcmp(&val, static_matrix_access(matrix, i, j),
                                sizeof(struct element8)));
      CATCH_REQUIRE(OK == static_matrix_clear(matrix, i, j));
      CATCH_REQUIRE(TRUE == ds_elt_zchk(static_matrix_access(matrix, i, j),
                                        sizeof(struct element8)));
    } /* for(j..) */
  } /* for(..) */

  static_matrix_destroy(matrix);
}

static void transpose_test(struct ds_params* params) {
  struct static_matrix *matrix;
  struct static_matrix mymatrix;

  matrix = static_matrix_init(&mymatrix, params);
  CATCH_REQUIRE(NULL != matrix);

  /*
   * If the # of rows and columns isn't equal, the transpose will fail, so
   * don't try
   */
  if (params->type.smat.n_rows != params->type.smat.n_cols) {
    static_matrix_destroy(matrix);
    return;
  }

  for (size_t i = 0; i < params->type.smat.n_rows; ++i) {
    for (size_t j = 0; j < params->type.smat.n_cols; ++j) {
      struct element8 val = { (int)i, (int)j };
      CATCH_REQUIRE(OK == static_matrix_set(matrix, i, j, &val));
      CATCH_REQUIRE(0 == memcmp(&val, static_matrix_access(matrix, i, j),
                                sizeof(struct element8)));
      CATCH_REQUIRE(OK == static_matrix_clear(matrix, i, j));
      CATCH_REQUIRE(TRUE == ds_elt_zchk(static_matrix_access(matrix, i, j),
                                        sizeof(struct element8)));
    } /* for(j..) */
  } /* for(..) */

  CATCH_REQUIRE(OK == static_matrix_transpose(matrix));

  for (size_t i = 0; i < params->type.smat.n_rows; ++i) {
    for (size_t j = 0; j < params->type.smat.n_cols; ++j) {
      struct element8* e1 = (element8*)static_matrix_access(matrix, i, j);
      struct element8* e2 = (element8*)static_matrix_access(matrix, j, i);

      CATCH_REQUIRE(e1->value1 == e2->value2);
      CATCH_REQUIRE(e2->value1 == e1->value2);

    } /* for(j..) */
  } /* for(..) */

  static_matrix_destroy(matrix);
}
