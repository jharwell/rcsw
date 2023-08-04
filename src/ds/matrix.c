/**
 * \file matrix.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/matrix.h"

#define RCSW_ER_MODNAME "rcsw.ds.matrix"
#define RCSW_ER_MODID ekLOG4CL_DS_MATRIX
#include "rcsw/er/client.h"
#include "rcsw/common/alloc.h"

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

struct matrix* matrix_init(struct matrix* const matrix_in,
                           const struct matrix_params* const params) {
  RCSW_FPC_NV(NULL,
              NULL != params,
              params->n_rows > 0,
              params->n_cols > 0)
  RCSW_ER_MODULE_INIT();

  struct matrix* matrix = rcsw_alloc(matrix_in,
                                     sizeof(struct matrix),
                                     params->flags & RCSW_NOALLOC_HANDLE);
  RCSW_CHECK_PTR(matrix);
  matrix->flags = params->flags;
  matrix->elt_size = params->elt_size;
  matrix->printe = params->printe;
  matrix->n_rows = params->n_rows;
  matrix->n_cols = params->n_cols;

  matrix->elements = rcsw_alloc(params->elements,
                                params->n_rows * params->n_cols * params->elt_size,
                                (params->flags & RCSW_NOALLOC_DATA) | RCSW_ZALLOC);

  RCSW_CHECK_PTR(matrix->elements);
  return matrix;

error:
  matrix_destroy(matrix);
  return NULL;
} /* matrix_init() */

void matrix_destroy(struct matrix* const matrix) {
  RCSW_FPC_V(NULL != matrix);

  rcsw_free(matrix->elements, matrix->flags & RCSW_NOALLOC_DATA);
  rcsw_free(matrix, matrix->flags & RCSW_NOALLOC_HANDLE);
} /* matrix_destroy() */

status_t matrix_transpose(struct matrix* const matrix) {
  RCSW_FPC_NV(ERROR, NULL != matrix, matrix_issquare(matrix));

  /*
   * Assuming matrix is square, the simple algorithm can be used. First and
   * last entries in matrix/array don't move, hence starting at 1.
   */
  ER_DEBUG("Transpose %zu x %zu matrix", matrix->n_rows, matrix->n_cols);
  for (size_t i = 1; i < matrix->n_rows; ++i) {
    for (size_t j = 0; j < i; ++j) {
      ds_elt_swap(matrix_access(matrix, i, j),
                  matrix_access(matrix, j, i),
                  matrix->elt_size);
    } /* for(j..) */
  } /* for(i..) */
  return OK;
} /* matrix_transpose() */

void matrix_print(const struct matrix* const matrix) {
  RCSW_FPC_V(NULL != matrix, NULL != matrix->printe);

  DPRINTF("{");
  for (size_t i = 0; i < matrix->n_rows; ++i) {
    DPRINTF("{");
    for (size_t j = 0; j < matrix->n_cols; ++j) {
      matrix->printe(matrix_access(matrix, i, j));
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
} /* matrix_print() */

END_C_DECLS
