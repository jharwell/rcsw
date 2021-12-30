/**
 * @file dynamic_matrix.c
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
#include "rcsw/ds/dynamic_matrix.h"
#include "rcsw/common/dbg.h"

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
#define MODULE_ID M_DS_DYNAMIC_MATRIX

/*******************************************************************************
 * Forward Declarations
 ******************************************************************************/
BEGIN_C_DECLS

/*******************************************************************************
 * API Functions
 ******************************************************************************/
struct dynamic_matrix* dynamic_matrix_init(struct dynamic_matrix* const matrix_in,
                                           const struct ds_params* const params) {
  RCSW_FPC_NV(NULL,
            NULL != params,
            params->tag == DS_DYNAMIC_MATRIX,
            params->type.dmat.n_rows > 0,
            params->type.dmat.n_cols > 0)
  struct dynamic_matrix* matrix = NULL;
  if (params->flags & DS_APP_DOMAIN_HANDLE) {
    RCSW_CHECK_PTR(matrix_in);
    matrix = matrix_in;
  } else {
    matrix = calloc(1, sizeof(struct dynamic_matrix));
    RCSW_CHECK_PTR(matrix);
  }
  matrix->flags = params->flags;
  matrix->el_size = params->el_size;
  matrix->printe = params->printe;
  matrix->n_rows = params->type.dmat.n_rows;
  matrix->n_cols = params->type.dmat.n_cols;

  struct ds_params handle_params = {.type = {.da = {.init_size = matrix->n_rows}},
                                    .cmpe = NULL,
                                    .printe = NULL,
                                    .nodes = NULL,
                                    .elements = NULL,
                                    .tag = DS_DARRAY,
                                    .el_size = sizeof(struct darray),
                                    .max_elts = -1,
                                    .flags = 0};
  matrix->rows = darray_init(NULL, &handle_params);
  RCSW_CHECK_PTR(matrix->rows);

  struct ds_params row_params = {.type = {.da = {.init_size = matrix->n_cols}},
                                 .cmpe = NULL,
                                 .printe = NULL,
                                 .nodes = NULL,
                                 .elements = NULL,
                                 .tag = DS_DARRAY,
                                 .el_size = matrix->el_size,
                                 .max_elts = -1,
                                 .flags = DS_APP_DOMAIN_HANDLE};

  for (size_t i = 0; i < matrix->n_rows; ++i) {
    RCSW_CHECK_PTR(darray_init(darray_data_get(matrix->rows, i), &row_params));
  } /* for(i..) */

  return matrix;

error:
  dynamic_matrix_destroy(matrix);
  return NULL;
} /* dynamic_matrix_init() */

void dynamic_matrix_destroy(struct dynamic_matrix* const matrix) {
  RCSW_FPC_V(NULL != matrix);

  for (size_t i = 0; i < matrix->n_rows; ++i) {
    darray_destroy(darray_data_get(matrix->rows, i));
  } /* for(i..) */
  darray_destroy(matrix->rows);

  if (!(matrix->flags & DS_APP_DOMAIN_HANDLE)) {
    free(matrix);
  }
} /* dynamic_matrix_destroy() */

status_t dynamic_matrix_set(struct dynamic_matrix* const matrix,
                            size_t u,
                            size_t v,
                            const void* const w) {
  RCSW_FPC_NV(ERROR, NULL != matrix);
  if (u >= matrix->n_rows || v >= matrix->n_cols) {
    RCSW_CHECK(OK == dynamic_matrix_resize(matrix, u + 1, v + 1));
  }
  ds_elt_copy(dynamic_matrix_access(matrix, u, v), w, matrix->el_size);
  return OK;

error:
  return ERROR;
} /* dynamic_matrix_set() */

status_t dynamic_matrix_resize(struct dynamic_matrix* const matrix,
                               size_t u,
                               size_t v) {
  RCSW_FPC_NV(ERROR, NULL != matrix);
  DBGD("Resizing matrix [%zu x %zu] -> [%zu x %zu]\n",
       matrix->n_rows,
       matrix->n_cols,
       RCSW_MAX(matrix->n_rows, u),
       RCSW_MAX(matrix->n_cols, v));
  if (u >= matrix->n_rows) {
    RCSW_CHECK(OK == darray_resize(matrix->rows, u));
    struct ds_params row_params = {.type = {.da = {.init_size = matrix->n_cols}},
                                   .cmpe = NULL,
                                   .printe = NULL,
                                   .nodes = NULL,
                                   .elements = NULL,
                                   .tag = DS_DARRAY,
                                   .el_size = matrix->el_size,
                                   .max_elts = -1,
                                   .flags = DS_APP_DOMAIN_HANDLE};

    for (size_t i = matrix->n_rows; i < u; ++i) {
      RCSW_CHECK_PTR(darray_init(darray_data_get(matrix->rows, i), &row_params));
    } /* for(i..) */
    matrix->n_rows = u;
  }
  if (v >= matrix->n_cols) {
    for (size_t i = 0; i < matrix->n_rows; ++i) {
      RCSW_CHECK(OK == darray_resize(darray_data_get(matrix->rows, i), v));
    } /* for(i..) */
    matrix->n_cols = v;
  }
  return OK;

error:
  return ERROR;
} /* dynamic_matrix_resize() */

status_t dynamic_matrix_transpose(struct dynamic_matrix* const matrix) {
  RCSW_FPC_NV(ERROR, NULL != matrix, matrix->n_rows == matrix->n_cols);

  /*
   * Assuming matrix is square, the simple algorithm can be used. First and
   * last entries in matrix/array don't move, hence starting at 1.
   */
  DBGD("Transpose %zu x %zu matrix\n", matrix->n_rows, matrix->n_cols);
  for (size_t i = 1; i < matrix->n_rows; ++i) {
    for (size_t j = 0; j < i; ++j) {
      ds_elt_swap(dynamic_matrix_access(matrix, i, j),
                  dynamic_matrix_access(matrix, j, i),
                  matrix->el_size);
    } /* for(j..) */
  }   /* for(i..) */
  return OK;
} /* dynamic_matrix_transpose() */

void dynamic_matrix_print(const struct dynamic_matrix* const matrix) {
  RCSW_FPC_V(NULL != matrix, NULL != matrix->printe);

  DPRINTF("{");
  for (size_t i = 0; i < matrix->n_rows; ++i) {
    DPRINTF("{");
    for (size_t j = 0; j < matrix->n_cols; ++j) {
      matrix->printe(dynamic_matrix_access(matrix, i, j));
      if (j < matrix->n_cols - 1) {
        DPRINTF(",");
      }
    } /* for(j..) */

    DPRINTF("}");
    if (i < matrix->n_rows - 1) {
      DPRINTF("\n");
    }
  } /* for(i..) */
  DPRINTF("}\n");
} /* dynamic_matrix_print() */

END_C_DECLS
