/**
 * \file dyn_matrix.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/dyn_matrix.h"

#define RCSW_ER_MODNAME "rcsw.ds.dyn_matrix"
#define RCSW_ER_MODID M_DS_DYN_MATRIX
#include "rcsw/er/client.h"

BEGIN_C_DECLS

/*******************************************************************************
 * API Functions
 ******************************************************************************/
struct dyn_matrix* dyn_matrix_init(struct dyn_matrix* const matrix_in,
                                   const struct ds_params* const params) {
  RCSW_FPC_NV(NULL,
              NULL != params,
              params->tag == ekRCSW_DS_DYN_MATRIX,
              params->type.dmat.n_rows > 0,
              params->type.dmat.n_cols > 0)
      RCSW_ER_MODULE_INIT();

  struct dyn_matrix* matrix = NULL;
  if (params->flags & RCSW_DS_NOALLOC_HANDLE) {
    RCSW_CHECK_PTR(matrix_in);
    matrix = matrix_in;
  } else {
    matrix = calloc(1, sizeof(struct dyn_matrix));
    RCSW_CHECK_PTR(matrix);
  }
  matrix->flags = params->flags;
  matrix->elt_size = params->elt_size;
  matrix->printe = params->printe;
  matrix->n_rows = params->type.dmat.n_rows;
  matrix->n_cols = params->type.dmat.n_cols;

  struct ds_params handle_params = { .type = { .da = { .init_size =
                                                           matrix->n_rows } },
                                     .cmpe = NULL,
                                     .printe = NULL,
                                     .nodes = NULL,
                                     .elements = NULL,
                                     .tag = ekRCSW_DS_DARRAY,
                                     .elt_size = sizeof(struct darray),
                                     .max_elts = -1,
                                     .flags = 0 };
  matrix->rows = darray_init(NULL, &handle_params);
  RCSW_CHECK_PTR(matrix->rows);

  struct ds_params row_params = { .type = { .da = { .init_size =
                                                        matrix->n_cols } },
                                  .cmpe = NULL,
                                  .printe = NULL,
                                  .nodes = NULL,
                                  .elements = NULL,
                                  .tag = ekRCSW_DS_DARRAY,
                                  .elt_size = matrix->elt_size,
                                  .max_elts = -1,
                                  .flags = RCSW_DS_NOALLOC_HANDLE };

  for (size_t i = 0; i < matrix->n_rows; ++i) {
    RCSW_CHECK_PTR(darray_init(darray_data_get(matrix->rows, i), &row_params));
  } /* for(i..) */

  return matrix;

error:
  dyn_matrix_destroy(matrix);
  return NULL;
} /* dyn_matrix_init() */

void dyn_matrix_destroy(struct dyn_matrix* const matrix) {
  RCSW_FPC_V(NULL != matrix);

  for (size_t i = 0; i < matrix->n_rows; ++i) {
    darray_destroy(darray_data_get(matrix->rows, i));
  } /* for(i..) */
  darray_destroy(matrix->rows);

  if (!(matrix->flags & RCSW_DS_NOALLOC_HANDLE)) {
    free(matrix);
  }
} /* dyn_matrix_destroy() */

status_t dyn_matrix_set(struct dyn_matrix* const matrix,
                            size_t u,
                            size_t v,
                            const void* const w) {
  RCSW_FPC_NV(ERROR, NULL != matrix);
  if (u >= matrix->n_rows || v >= matrix->n_cols) {
    RCSW_CHECK(OK == dyn_matrix_resize(matrix, u + 1, v + 1));
  }
  ds_elt_copy(dyn_matrix_access(matrix, u, v), w, matrix->elt_size);
  return OK;

error:
  return ERROR;
} /* dyn_matrix_set() */

status_t dyn_matrix_resize(struct dyn_matrix* const matrix,
                           size_t u,
                           size_t v) {
  RCSW_FPC_NV(ERROR, NULL != matrix);

  ER_DEBUG("Resizing matrix [%zu x %zu] -> [%zu x %zu]",
       matrix->n_rows,
       matrix->n_cols,
       RCSW_MAX(matrix->n_rows, u),
       RCSW_MAX(matrix->n_cols, v));

  if (u >= matrix->n_rows) {
    RCSW_CHECK(OK == darray_resize(matrix->rows, u));
    struct ds_params row_params = { .type = { .da = { .init_size =
                                                          matrix->n_cols } },
                                    .cmpe = NULL,
                                    .printe = NULL,
                                    .nodes = NULL,
                                    .elements = NULL,
                                    .tag = ekRCSW_DS_DARRAY,
                                    .elt_size = matrix->elt_size,
                                    .max_elts = -1,
                                    .flags = RCSW_DS_NOALLOC_HANDLE };

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
} /* dyn_matrix_resize() */

status_t dyn_matrix_transpose(struct dyn_matrix* const matrix) {
  RCSW_FPC_NV(ERROR, NULL != matrix, dyn_matrix_issquare(matrix));

  /*
   * Assuming matrix is square, the simple algorithm can be used. First and
   * last entries in matrix/array don't move, hence starting at 1.
   */
  ER_DEBUG("Transpose %zu x %zu matrix", matrix->n_rows, matrix->n_cols);
  for (size_t i = 1; i < matrix->n_rows; ++i) {
    for (size_t j = 0; j < i; ++j) {
      ds_elt_swap(dyn_matrix_access(matrix, i, j),
                  dyn_matrix_access(matrix, j, i),
                  matrix->elt_size);
    } /* for(j..) */
  } /* for(i..) */
  return OK;
} /* dyn_matrix_transpose() */

void dyn_matrix_print(const struct dyn_matrix* const matrix) {
  RCSW_FPC_V(NULL != matrix, NULL != matrix->printe);

  DPRINTF("{");
  for (size_t i = 0; i < matrix->n_rows; ++i) {
    DPRINTF("{");
    for (size_t j = 0; j < matrix->n_cols; ++j) {
      matrix->printe(dyn_matrix_access(matrix, i, j));
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
} /* dyn_matrix_print() */

END_C_DECLS
