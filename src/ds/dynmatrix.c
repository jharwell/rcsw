/**
 * \file
 *
 * \copyright 2017 John Harwell
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/dynmatrix.h"

#include <string.h>

#define RCSW_ER_MODNAME RCSW_ER_MODNAME_BUILDER("rcsw", "ds", "dynmatrix")
#define RCSW_ER_MODID LOG4CL_DS_DYNMATRIX
#include "rcsw/core/alloc.h"
#include "rcsw/core/core.h"
#include "rcsw/er/client.h"

/*******************************************************************************
 * Public API
 ******************************************************************************/
BEGIN_C_DECLS

struct dynmatrix* dynmatrix_init(struct dynmatrix* const              matrix_in,
                                 const struct dynmatrix_config* const params) {
  RCSW_FPC_NV(NULL,
              NULL != params,
              params->n_rows > 0,
              params->n_cols > 0,
              params->elt_size > 0);
  RCSW_ER_MODULE_INIT();

  struct dynmatrix* matrix =
    rcsw_alloc(matrix_in,
               sizeof(struct dynmatrix),
               params->flags & (RCSW_NOALLOC_HANDLE | RCSW_ZALLOC));

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
  /*
   * darray_init() always sets current=0 (init_size only controls capacity).
   * Pre-mark all row slots accessible so darray_data_get() bounds checks
   * pass during the init loop below and all subsequent dynmatrix operations.
   */
  matrix->rows->current = matrix->n_rows;

  struct darray_config row_params = {.init_size = matrix->n_cols,
                                     .cmpe      = NULL,
                                     .printe    = NULL,
                                     .elements  = NULL,
                                     .elt_size  = matrix->elt_size,
                                     .max_elts  = -1,
                                     .flags = RCSW_NOALLOC_HANDLE | RCSW_ZALLOC};

  for (size_t i = 0; i < matrix->n_rows; ++i) {
    struct darray* row = (struct darray*)darray_data_get(matrix->rows, i);
    RCSW_CHECK_PTR(darray_init(row, &row_params));
    /* Same: pre-mark all column slots accessible. */
    row->current = matrix->n_cols;
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
  RCSW_FPC_NV(ERROR, NULL != matrix, NULL != w);
  if (u >= matrix->n_rows || v >= matrix->n_cols) {
    RCSW_CHECK(OK == dynmatrix_resize(matrix,
                                      RCSW_MAX(matrix->n_rows, u + 1),
                                      RCSW_MAX(matrix->n_cols, v + 1)));
  }
  memcpy(dynmatrix_access(matrix, u, v), w, matrix->elt_size);
  return OK;

error:
  return ERROR;
} /* dynmatrix_set() */

/**
 * \brief Resize one row to exactly \p n_cols columns, zeroing new cells.
 */
static status_t dynmatrix_row_resize(struct dynmatrix* const matrix,
                                     struct darray* const    row,
                                     size_t                  n_cols) {
  size_t old_cols = darray_size(row);
  if (n_cols > darray_capacity(row) || n_cols < old_cols) {
    RCSW_CHECK(OK == darray_resize(row, n_cols));
  }
  if (n_cols > old_cols) {
    memset(row->elements + (old_cols * matrix->elt_size),
           0,
           (n_cols - old_cols) * matrix->elt_size);
  }
  return darray_set_size(row, n_cols);

error:
  return ERROR;
} /* dynmatrix_row_resize() */

status_t dynmatrix_resize(struct dynmatrix* const matrix, size_t u, size_t v) {
  RCSW_FPC_NV(ERROR, NULL != matrix, u > 0, v > 0);

  ER_DEBUG("Resizing matrix [%zu x %zu] -> [%zu x %zu]",
           matrix->n_rows,
           matrix->n_cols,
           u,
           v);

  /* Rows going away */
  for (size_t i = u; i < matrix->n_rows; ++i) {
    darray_destroy(darray_data_get(matrix->rows, i));
  } /* for(i..) */
  size_t kept_rows = RCSW_MIN(u, matrix->n_rows);
  if (u > darray_capacity(matrix->rows) || u < matrix->n_rows) {
    RCSW_CHECK(OK == darray_resize(matrix->rows, u));
  }
  RCSW_CHECK(OK == darray_set_size(matrix->rows, u));
  matrix->n_rows = kept_rows;

  /* Surviving rows: adjust the column count */
  for (size_t i = 0; i < kept_rows; ++i) {
    RCSW_CHECK(OK ==
               dynmatrix_row_resize(matrix, darray_data_get(matrix->rows, i), v));
  } /* for(i..) */

  /* New rows: zero-filled, v columns */
  struct darray_config row_params = {.init_size = v,
                                     .cmpe      = NULL,
                                     .printe    = NULL,
                                     .elements  = NULL,
                                     .elt_size  = matrix->elt_size,
                                     .max_elts  = -1,
                                     .flags = RCSW_NOALLOC_HANDLE | RCSW_ZALLOC};
  for (size_t i = kept_rows; i < u; ++i) {
    struct darray* row = darray_data_get(matrix->rows, i);
    RCSW_CHECK_PTR(darray_init(row, &row_params));
    RCSW_CHECK(OK == darray_set_size(row, v));
    matrix->n_rows = i + 1;
  } /* for(i..) */

  matrix->n_rows = u;
  matrix->n_cols = v;
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
  RCSW_FPC_V(NULL != matrix);
  ER_ASSERT(NULL != matrix->printe, "dynmatrix_print() requires printe()");

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
