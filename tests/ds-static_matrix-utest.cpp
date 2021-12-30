/**
 * @file static_matrix-test.cpp
 *
 * Test of static matrix.
 *
 * @copyright 2017 John Harwell, All rights reserved.
 *
 * This file is part of RCSW.
 *
 * RCSW is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * RCSW is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * RCSW.  If not, see <http://www.gnu.org/licenses/
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdlib.h>
#include <limits.h>
#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_PREFIX_ALL
#include <catch.hpp>
#include "rcsw/ds/static_matrix.h"
#include "rcsw/common/dbg.h"
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
  dbg_init();
  dbg_insmod(M_TESTING,"Testing");
  dbg_insmod(M_DS_STATIC_MATRIX,"STATIC_MATRIX");
  dbg_mod_lvl_set(M_DS_STATIC_MATRIX, DBG_V);

  struct ds_params params;
  params.tag = DS_STATIC_MATRIX;
  params.el_size = sizeof(struct element);
  params.type.smat.n_cols = NUM_ITEMS;
  params.type.smat.n_rows = NUM_ITEMS;
  CATCH_REQUIRE(th_ds_init(&params) == OK);

  for (size_t i = 1; i <= 10; ++i) {
    for (size_t j = 1; j <= 10; ++j) {
      params.type.smat.n_cols = i;
      params.type.smat.n_rows = j;
      params.flags = 0;
      test(&params);
      params.flags = DS_APP_DOMAIN_DATA;
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
      struct element val = { rand(), rand() };
  CATCH_REQUIRE(OK == static_matrix_set(matrix, i, j, &val));
      CATCH_REQUIRE(0 == memcmp(&val, static_matrix_access(matrix, i, j),
                                sizeof(struct element)));
      CATCH_REQUIRE(OK == static_matrix_clear(matrix, i, j));
      CATCH_REQUIRE(TRUE == ds_elt_zchk(static_matrix_access(matrix, i, j),
                                        sizeof(struct element)));
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
      struct element val = { (int)i, (int)j };
      CATCH_REQUIRE(OK == static_matrix_set(matrix, i, j, &val));
      CATCH_REQUIRE(0 == memcmp(&val, static_matrix_access(matrix, i, j),
                                sizeof(struct element)));
      CATCH_REQUIRE(OK == static_matrix_clear(matrix, i, j));
      CATCH_REQUIRE(TRUE == ds_elt_zchk(static_matrix_access(matrix, i, j),
                                        sizeof(struct element)));
    } /* for(j..) */
  } /* for(..) */

  CATCH_REQUIRE(OK == static_matrix_transpose(matrix));

  for (size_t i = 0; i < params->type.smat.n_rows; ++i) {
    for (size_t j = 0; j < params->type.smat.n_cols; ++j) {
      struct element* e1 = (element*)static_matrix_access(matrix, i, j);
      struct element* e2 = (element*)static_matrix_access(matrix, j, i);

      CATCH_REQUIRE(e1->value1 == e2->value2);
      CATCH_REQUIRE(e2->value1 == e1->value2);

    } /* for(j..) */
  } /* for(..) */

  static_matrix_destroy(matrix);
}
