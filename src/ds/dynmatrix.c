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
#include "rcsw/utils/byteops.h"

/*******************************************************************************
 * Public API
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Release a (possibly partially initialized) dynmatrix.
 *
 * Only the first matrix->n_rows rows are destroyed, so n_rows must count the
 * rows that have actually been initialized.
 */
static void dynmatrix_teardown(struct dynmatrix* const matrix) {
  if (NULL == matrix) {
    return;
  }
  if (NULL != matrix->rows) {
    for (size_t i = 0; i < matrix->n_rows; ++i) {
      darray_destroy(darray_data_get(matrix->rows, i));
    } /* for(i..) */
    darray_destroy(matrix->rows);
  }
  rcsw_free(matrix, matrix->flags & RCSW_NOALLOC_HANDLE);
} /* dynmatrix_teardown() */

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
  matrix->rows     = NULL;
  /* counts initialized rows, so a failure part way through can be undone */
  matrix->n_rows = 0;
  matrix->n_cols = params->n_cols;

  struct darray_config handle_params = {.init_size = params->n_rows,
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
  matrix->rows->current = params->n_rows;

  struct darray_config row_params = {.init_size = matrix->n_cols,
                                     .cmpe      = NULL,
                                     .printe    = NULL,
                                     .elements  = NULL,
                                     .elt_size  = matrix->elt_size,
                                     .max_elts  = -1,
                                     .flags = RCSW_NOALLOC_HANDLE | RCSW_ZALLOC};

  for (size_t i = 0; i < params->n_rows; ++i) {
    struct darray* row = (struct darray*)darray_data_get(matrix->rows, i);
    RCSW_CHECK_PTR(darray_init(row, &row_params));
    /* Same: pre-mark all column slots accessible. */
    row->current = matrix->n_cols;
    matrix->n_rows++;
  } /* for(i..) */

  return matrix;

error:
  /*
   * Rows past n_rows were never initialized: they are zero-filled, and
   * destroying one would free() a pointer into the middle of the row array.
   */
  dynmatrix_teardown(matrix);
  return NULL;
} /* dynmatrix_init() */

void dynmatrix_destroy(struct dynmatrix* const matrix) {
  RCSW_FPC_V(NULL != matrix);
  dynmatrix_teardown(matrix);
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

status_t dynmatrix_resize(struct dynmatrix* const matrix, size_t u, size_t v) {
  RCSW_FPC_NV(ERROR, NULL != matrix, u > 0, v > 0);

  ER_DEBUG("Resizing matrix [%zu x %zu] -> [%zu x %zu]",
           matrix->n_rows,
           matrix->n_cols,
           u,
           v);

  size_t old_rows  = matrix->n_rows;
  size_t kept_rows = RCSW_MIN(u, old_rows);
  size_t n_new     = 0; /* new rows initialized so far */

  /*
   * Phase 1: every step that can fail. Until phase 2 the matrix's contents and
   * dimensions are unchanged; a failure here leaves at most some extra
   * capacity reserved, so nothing needs to be undone except new rows.
   */
  if (u > darray_capacity(matrix->rows)) {
    RCSW_CHECK(OK == darray_resize(matrix->rows, u));
  }
  for (size_t i = 0; i < kept_rows; ++i) {
    struct darray* row = darray_data_get(matrix->rows, i);
    if (v > darray_capacity(row)) {
      RCSW_CHECK(OK == darray_resize(row, v));
    }
  } /* for(i..) */

  /* New rows: zero-filled, v columns, in slots the row array already has */
  struct darray_config row_params = {.init_size = v,
                                     .cmpe      = NULL,
                                     .printe    = NULL,
                                     .elements  = NULL,
                                     .elt_size  = matrix->elt_size,
                                     .max_elts  = -1,
                                     .flags = RCSW_NOALLOC_HANDLE | RCSW_ZALLOC};
  if (u > old_rows) {
    /* within capacity, so cannot fail; makes the new slots addressable */
    darray_set_size(matrix->rows, u);
    for (size_t i = old_rows; i < u; ++i) {
      struct darray* row = darray_data_get(matrix->rows, i);
      if (NULL == darray_init(row, &row_params)) {
        goto undo_new_rows;
      }
      darray_set_size(row, v); /* within capacity */
      ++n_new;
    } /* for(i..) */
  }

  /*
   * Phase 2: commit. Nothing here can fail: every size set is within the
   * capacity reserved above.
   */
  for (size_t i = 0; i < kept_rows; ++i) {
    struct darray* row      = darray_data_get(matrix->rows, i);
    size_t         old_cols = darray_size(row);
    if (v > old_cols) {
      memset(row->elements + (old_cols * matrix->elt_size),
             0,
             (v - old_cols) * matrix->elt_size);
    }
    darray_set_size(row, v);
  } /* for(i..) */

  /* Rows going away are destroyed only now, so a failure above loses no data */
  for (size_t i = u; i < old_rows; ++i) {
    darray_destroy(darray_data_get(matrix->rows, i));
  } /* for(i..) */
  darray_set_size(matrix->rows, u);
  matrix->n_rows = u;
  matrix->n_cols = v;

  /*
   * Best effort: give back capacity that is no longer needed. A failed shrink
   * leaves the array as it was, so the result can be ignored.
   */
  for (size_t i = 0; i < kept_rows; ++i) {
    struct darray* row = darray_data_get(matrix->rows, i);
    if (darray_capacity(row) > v) {
      (void)darray_resize(row, v);
    }
  } /* for(i..) */
  if (darray_capacity(matrix->rows) > u) {
    (void)darray_resize(matrix->rows, u);
  }
  return OK;

undo_new_rows:
  for (size_t i = old_rows; i < old_rows + n_new; ++i) {
    darray_destroy(darray_data_get(matrix->rows, i));
  } /* for(i..) */
  darray_set_size(matrix->rows, old_rows);

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
      utils_elt_swap(dynmatrix_access(matrix, i, j),
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
