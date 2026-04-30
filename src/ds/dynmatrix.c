/**
 * \file
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/dynmatrix.h"

#define RCSW_ER_MODNAME RCSW_ER_MODNAME_BUILDER("rcsw", "ds", "dynmatrix")
#define RCSW_ER_MODID ekLOG4CL_DS_DYNMATRIX
#include "rcsw/core/alloc.h"
#include "rcsw/er/client.h"

/*******************************************************************************
 * Public API
 ******************************************************************************/
BEGIN_C_DECLS

struct dynmatrix* dynmatrix_init(struct dynmatrix* const              matrix_in,
                                 const struct dynmatrix_config* const params) {
  RCSW_FPC_NV(NULL, NULL != params, params->n_rows > 0, params->n_cols > 0)
  RCSW_ER_MODULE_INIT();

  struct dynmatrix* matrix = rcsw_alloc(matrix_in,
                                        sizeof(struct dynmatrix),
                                        params->flags & RCSW_NOALLOC_HANDLE);

  RCSW_CHECK_PTR(matrix);
  matrix->flags    = params->flags;
  matrix->elt_size = params->elt_size;
  matrix->printe   = params->printe;
  matrix->n_rows   = params->n_rows;
  matrix->n_cols   = params->n_cols;

  struct darray_config handle_params = {.init_size = matrix->n_rows,
                                        .cmpe      = NULL,
                                        .printe    = NULL,
                                        .elements  = NULL,
                                        .elt_size  = sizeof(struct darray),
                                        .max_elts  = -1,
                                        .flags     = RCSW_ZALLOC};
  matrix->rows                       = darray_init(NULL, &handle_params);
  RCSW_CHECK_PTR(matrix->rows);

  struct darray_config row_params = {.init_size = matrix->n_cols,
                                     .cmpe      = NULL,
                                     .printe    = NULL,
                                     .elements  = NULL,
                                     .elt_size  = matrix->elt_size,
                                     .max_elts  = -1,
                                     .flags = RCSW_NOALLOC_HANDLE | RCSW_ZALLOC};

  for (size_t i = 0; i < matrix->n_rows; ++i) {
    RCSW_CHECK_PTR(darray_init(darray_data_get(matrix->rows, i), &row_params));
  } /* for(i..) */

  return matrix;

error:
  dynmatrix_destroy(matrix);
  return NULL;
} /* dynmatrix_init() */

void dynmatrix_destroy(struct dynmatrix* const matrix) {
  RCSW_FPC_V(NULL != matrix);

  for (size_t i = 0; i < matrix->n_rows; ++i) {
    darray_destroy(darray_data_get(matrix->rows, i));
  } /* for(i..) */
  darray_destroy(matrix->rows);

  rcsw_free(matrix, matrix->flags & RCSW_NOALLOC_HANDLE);
} /* dynmatrix_destroy() */

status_t dynmatrix_set(struct dynmatrix* const matrix,
                       size_t                  u,
                       size_t                  v,
                       const void* const       w) {
  RCSW_FPC_NV(ERROR, NULL != matrix);
  if (u >= matrix->n_rows || v >= matrix->n_cols) {
    RCSW_CHECK(OK == dynmatrix_resize(matrix, u + 1, v + 1));
  }
  ds_elt_copy(dynmatrix_access(matrix, u, v), w, matrix->elt_size);
  return OK;

error:
  return ERROR;
} /* dynmatrix_set() */

status_t dynmatrix_resize(struct dynmatrix* const matrix, size_t u, size_t v) {
  RCSW_FPC_NV(ERROR, NULL != matrix);

  ER_DEBUG("Resizing matrix [%zu x %zu] -> [%zu x %zu]",
           matrix->n_rows,
           matrix->n_cols,
           RCSW_MAX(matrix->n_rows, u),
           RCSW_MAX(matrix->n_cols, v));

  if (u >= matrix->n_rows) {
    RCSW_CHECK(OK == darray_resize(matrix->rows, u));
    struct darray_config row_params = {.init_size = matrix->n_cols,
                                       .cmpe      = NULL,
                                       .printe    = NULL,
                                       .elements  = NULL,
                                       .elt_size  = matrix->elt_size,
                                       .max_elts  = -1,
                                       .flags     = RCSW_NOALLOC_HANDLE};

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
} /* dynmatrix_resize() */

status_t dynmatrix_transpose(struct dynmatrix* const matrix) {
  RCSW_FPC_NV(ERROR, NULL != matrix, dynmatrix_issquare(matrix));

  /*
   * Assuming matrix is square, the simple algorithm can be used. First and
   * last entries in matrix/array don't move, hence starting at 1.
   */
  ER_DEBUG("Transpose %zu x %zu matrix", matrix->n_rows, matrix->n_cols);
  for (size_t i = 1; i < matrix->n_rows; ++i) {
    for (size_t j = 0; j < i; ++j) {
      ds_elt_swap(dynmatrix_access(matrix, i, j),
                  dynmatrix_access(matrix, j, i),
                  matrix->elt_size);
    } /* for(j..) */
  } /* for(i..) */
  return OK;
} /* dynmatrix_transpose() */

void dynmatrix_print(const struct dynmatrix* const matrix) {
  RCSW_FPC_V(NULL != matrix, NULL != matrix->printe);

  DPRINTF("{");
  for (size_t i = 0; i < matrix->n_rows; ++i) {
    DPRINTF("{");
    for (size_t j = 0; j < matrix->n_cols; ++j) {
      matrix->printe(dynmatrix_access(matrix, i, j));
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
}

END_C_DECLS
