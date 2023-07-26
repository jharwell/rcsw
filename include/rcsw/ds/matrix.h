/**
 * \file matrix.h
 * \ingroup ds
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <math.h>
#include "rcsw/ds/ds.h"
#include "rcsw/common/fpc.h"

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/
/**
 * \brief Static matrix initialization parameters.
 */
struct matrix_params {
  /**
   * For printing an element. Can be NULL. If NULL, \ref matrix_print() is
   * disabled.
   */
  void (*printe)(const void *e);


  /**
   * Pointer to application-allocated space for storing the data managed by the
   * \ref matrix. Ignored unless \ref RCSW_NOALLOC_DATA is passed.
   */
  uint8_t *elements;

  /**
   * Size of elements in bytes.
   */
  size_t elt_size;

  /**
   * Configuration flags. See \ref matrix.flags for valid flags.
   */
  uint32_t flags;

  /**
   * Number of rows in matrix.
   */
  size_t n_rows;

  /**
   * Number of columns in matrix.
   */
  size_t n_cols;
};

/**
 * \brief Representation of a fixed-size (static) matrix using a single
 * contiguous array and indexing said array in 2 dimensions.
 *
 * Static in the sense that the matrix dimensions cannot be changed after
 * initialization. Assumes row-major order (this is C after all).
 *
 * Should only be used for applications where the matrix is dense, or when the
 * dimensions of the matrix are small (if the dimensions are large AND the
 * matrix is sparse, it will definitely be really inefficient, and possibly
 * won't even fit into memory).
 */
struct matrix {
  /** # of rows in matrix. */
  size_t n_rows;

  /** # of columns in matrix. */
  size_t n_cols;

  /** Matrix data. */
  uint8_t* elements;

  /**
   * Run-time configuration flags. Valid flags are:
   *
   * - \ref RCSW_NOALLOC_HANDLE
   * - \ref RCSW_NOALLOC_DATA
   *
   * All other flags ignored.
   */
  uint32_t flags;

  /** Size of matrix elements in bytes. */
  size_t elt_size;

  /** For printing an element. Can be NULL. */
  void (*printe)(const void *const e);
};

/*******************************************************************************
 * Inline Functions
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Access an element within a static matrix.
 *
 * \param matrix The matrix handle.
 * \param u The row within the matrix.
 * \param v The column within the matrix.
 *
 * \return Reference to element, or NULL if an error occurred.
 */
static inline void* matrix_access(const struct matrix* const matrix,
                                         size_t u,
                                         size_t v) {
  RCSW_FPC_NV(NULL,
              NULL != matrix,
              u < matrix->n_rows,
            v < matrix->n_cols);
  return matrix->elements + (matrix->n_cols * matrix->elt_size * u) +
      (matrix->elt_size * v);
}

/**
 * \brief Calculate the # of bytes required for the static matrix.
 *
 * \param n_rows Initial # rows.
 * \param n_cols Initial # columns.
 * \param elt_size Size of the elements in bytes.
 *
 * \return The # of bytes required.
 */
static inline size_t matrix_element_space(size_t n_rows,
                                                 size_t n_cols,
                                                 size_t elt_size) {
  return ds_elt_space_simple(n_rows * n_cols, elt_size);
}

/**
 * \brief Clear an element within the static matrix.
 *
 * \param matrix The matrix handle.
 * \param u Row of element to clear.
 * \param v Column of element to clear.
 *
 * \return \ref status_t
 */
static inline status_t matrix_elt_clear(struct matrix* const matrix,
                                               size_t u,
                                               size_t v) {
  RCSW_FPC_NV(ERROR, NULL != matrix, u < matrix->n_rows, v < matrix->n_cols);
  ds_elt_clear(matrix_access(matrix, u, v), matrix->elt_size);
  return OK;
}

/**
 * \brief Set an element in the static matrix to a specific value.
 *
 * \param matrix The matrix handle.
 * \param u Element row index.
 * \param v Element column index.
 * \param w New element value.
 *
 * \return \ref status_t
 */
static inline status_t matrix_set(struct matrix* const matrix,
                                         size_t u,
                                         size_t v,
                                         const void *const w) {
  RCSW_FPC_NV(ERROR, NULL != matrix, NULL != w, u < matrix->n_rows,
            v < matrix->n_cols);

  ds_elt_copy(matrix_access(matrix, u, v), w, matrix->elt_size);
  return OK;
} /* matrix_set() */

/**
 * \brief Return if the \ref matrix is square or not.
 */
static inline bool_t matrix_issquare(const struct matrix* const matrix) {
  RCSW_FPC_NV(false, NULL != matrix);
  return matrix->n_cols == matrix->n_rows;
}
/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
/**
 * \brief Initialize a static matrix.
 *
 * \param matrix_in An application allocated handle for the static matrix. Can
 *                  be NULL if \ref RCSW_NOALLOC_HANDLE is passed as a flag..
 *
 * \param params The initialization parameters.
 *
 * \return The initialized matrix, or NULL if an error occurred.
 */
struct matrix* matrix_init(struct matrix* matrix_in,
                           const struct matrix_params* params);

/**
 * \brief Destroy a static matrix.
 *
 * Any further use of the handle is undefined after this function is called.
 *
 * \param matrix The matrix handle.
 */
void matrix_destroy(struct matrix* matrix);

/**
 * \brief Transpose a static matrix.
 *
 * \note Currently only works if the matrix is square.
 *
 * \param matrix The matrix handle.
 *
 * \return \ref status_t
 */
status_t matrix_transpose(struct matrix* matrix);

/**
 * \brief Print a static matrix
 *
 * Calls the printe() function on each element in the matrix.
 *
 * \param matrix The matrix handle.
 */
void matrix_print(const struct matrix* matrix);

END_C_DECLS
