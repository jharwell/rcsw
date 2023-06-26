/**
 * \file static_matrix.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/static_matrix.h"

#define RCSW_ER_MODNAME "rcsw.ds.static_mat"
#define RCSW_ER_MODID M_DS_STATIC_MATRIX
#include "rcsw/er/client.h"

/*******************************************************************************
 * Forward Declarations
 ******************************************************************************/
BEGIN_C_DECLS

/*******************************************************************************
 * API Functions
 ******************************************************************************/
struct static_matrix* static_matrix_init(struct static_matrix* const matrix_in,
                                         const struct ds_params* const params) {
  RCSW_FPC_NV(NULL,
              NULL != params,
              params->tag == ekRCSW_DS_STATIC_MATRIX,
              params->type.smat.n_rows > 0,
              params->type.smat.n_cols > 0)
      RCSW_ER_MODULE_INIT();
  struct static_matrix* matrix = NULL;
  if (params->flags & RCSW_DS_NOALLOC_HANDLE) {
    RCSW_CHECK_PTR(matrix_in);
    matrix = matrix_in;
  } else {
    matrix = malloc(sizeof(struct static_matrix));
    RCSW_CHECK_PTR(matrix);
  }
  matrix->flags = params->flags;
  matrix->elt_size = params->elt_size;
  matrix->printe = params->printe;
  matrix->n_rows = params->type.smat.n_rows;
  matrix->n_cols = params->type.smat.n_cols;

  if (matrix->flags & RCSW_DS_NOALLOC_DATA) {
    matrix->elements = params->elements;
  } else {
    matrix->elements = calloc(matrix->n_rows * matrix->n_cols, matrix->elt_size);
  }
  RCSW_CHECK_PTR(matrix->elements);
  return matrix;

error:
  static_matrix_destroy(matrix);
  return NULL;
} /* static_matrix_init() */

void static_matrix_destroy(struct static_matrix* const matrix) {
  RCSW_FPC_V(NULL != matrix);
  if (!(matrix->flags & RCSW_DS_NOALLOC_DATA)) {
    free(matrix->elements);
  }
  if (!(matrix->flags & RCSW_DS_NOALLOC_HANDLE)) {
    free(matrix);
  }
} /* static_matrix_destroy() */

status_t static_matrix_transpose(struct static_matrix* const matrix) {
  RCSW_FPC_NV(ERROR, NULL != matrix, matrix->n_rows == matrix->n_cols);

  /*
   * Assuming matrix is square, the simple algorithm can be used. First and
   * last entries in matrix/array don't move, hence starting at 1.
   */
  ER_DEBUG("Transpose %zu x %zu matrix", matrix->n_rows, matrix->n_cols);
  for (size_t i = 1; i < matrix->n_rows; ++i) {
    for (size_t j = 0; j < i; ++j) {
      ds_elt_swap(static_matrix_access(matrix, i, j),
                  static_matrix_access(matrix, j, i),
                  matrix->elt_size);
    } /* for(j..) */
  } /* for(i..) */
  return OK;
} /* static_matrix_transpose() */

void static_matrix_print(const struct static_matrix* const matrix) {
  RCSW_FPC_V(NULL != matrix, NULL != matrix->printe);

  DPRINTF("{");
  for (size_t i = 0; i < matrix->n_rows; ++i) {
    DPRINTF("{");
    for (size_t j = 0; j < matrix->n_cols; ++j) {
      matrix->printe(static_matrix_access(matrix, i, j));
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
} /* static_matrix_print() */

END_C_DECLS
