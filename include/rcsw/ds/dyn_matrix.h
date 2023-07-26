/**
 * \file dyn_matrix.h
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
#include "rcsw/ds/darray.h"
#include "rcsw/ds/llist.h"

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/
/**
 * \brief Dynamic matrix initialization parameters.
 */
struct dyn_matrix_params {
  /**
   * For printing an element. Can be NULL.
   */
  void (*printe)(const void *e);

  /**
   * Size of elements in bytes.
   */
  size_t elt_size;

  /**
   * Configuration flags. See \ref dyn_matrix.flags for valid flags.
   */
  uint32_t flags;

  size_t n_rows;  /// # rows in matrix.
  size_t n_cols;  /// # columns in matrix.
  uint8_t* rows;  /// Ptr to space for vector-of-row-vectors.
};


/**
 * \brief Representation of a dynamically-sized matrix using a dynamic array row
 * vectors, with each row vector also being a dynamic array.
 *
 * Dynamic in the sense that the matrix dimensions can be changed after
 * initialization. The matrix can grow to be any size, but an initial size can
 * be specified during initialization, as the resizing operation can be very
 * expensive, depending on the current size of the matrix. Assumes row-major
 * order (this is C after all).
 *
 * Should only be used for applications where the matrix is dense, or when the
 * dimensions of the matrix are small (if the dimensions are large AND the
 * matrix is sparse, it will definitely be really inefficient, and possibly
 * won't even fit into memory). In addition, this data structure should really
 * only be used if you can guarantee that resizing the matrix happens
 * infrequently.
 *
 * The matrix is never resized autonomously.
 */
struct dyn_matrix {
  /**
   * Vector of row vectors.
   */
  struct darray* rows;

  /**
   * Number of rows in matrix.
   */
  size_t n_rows;

  /**
   * Number of columns in matrix.
   */
  size_t n_cols;

  /**
   * Size of matrix elements in bytes.
   */
  size_t elt_size;

  /**
   * Configuration flags. Valid flags are:
   *
   * - \ref RCSW_NOALLOC_HANDLE
   *
   * All other flags are ignored.
   */
  uint32_t flags;

  /**
   * For printing an element. Can be NULL. If NULL, then \ref
   * dyn_matrix_print() is disabled.
   */
  void (*printe)(const void *const e);
};

/*******************************************************************************
 * Inline Functions
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Access an element within a dynamic matrix.
 *
 * The element must exist within the matrix (i.e. this function does not expand
 * the matrix).
 *
 * \param matrix The matrix handle.
 * \param u The row within the matrix.
 * \param v The column within the matrix.
 *
 * \return Reference to element, or NULL if an error occurred.
 */
static inline void* dyn_matrix_access(const struct dyn_matrix* const matrix,
                                          size_t u, size_t v) {
    RCSW_FPC_NV(NULL,
                NULL != matrix,
                u < matrix->n_rows,
              v < matrix->n_cols);
    return darray_data_get((struct darray*)darray_data_get(matrix->rows, u), v);
}

/**
 * \brief Calculate the # of bytes required for the initial size of the dynamic
 * matrix.
 *
 * \param n_rows Initial # rows.
 * \param n_cols Initial # columns.
 * \param elt_size Size of the elements in bytes.
 *
 * \return The # of bytes required.
 */
static inline size_t dyn_matrix_space(size_t n_rows,
                                      size_t n_cols,
                                      size_t elt_size) {
  return darray_element_space(n_cols, elt_size) * n_rows +
      darray_element_space(n_rows, sizeof(struct darray));
}

/**
 * \brief Clear an element within the dynamic matrix.
 *
 * \param matrix The matrix handle.
 * \param u Row of element to clear.
 * \param v Column of element to clear.
 *
 * \return \ref status_t
 */
static inline status_t dyn_matrix_clear(struct dyn_matrix* const matrix,
                                           size_t u, size_t v) {
    RCSW_FPC_NV(ERROR,
                NULL != matrix,
                u < matrix->n_rows,
                v < matrix->n_cols);
    ds_elt_clear(dyn_matrix_access(matrix, u, v), matrix->elt_size);
    return OK;
}

/**
 * \brief Return if the \ref dyn_matrix is square or not.
 */
static inline bool_t dyn_matrix_issquare(const struct dyn_matrix* const matrix) {
  RCSW_FPC_NV(false, NULL != matrix);

  return matrix->n_cols == matrix->n_rows;
}

/*******************************************************************************
 * API Functions
 ******************************************************************************/
/**
 * \brief Initialize a dynamic matrix.
 *
 * \param matrix_in An application allocated handle for the dynamic matrix. Can
 *                  be NULL if \ref RCSW_NOALLOC_HANDLE is passed as a flag.
 *
 * \param params The initialization parameters.
 *
 * \return The initialized matrix, or NULL if an error occurred.
 */
struct dyn_matrix* dyn_matrix_init(struct dyn_matrix* matrix_in,
                                   const struct dyn_matrix_params* params) RCSW_CHECK_RET;

/**
 * \brief Destroy a dynamic matrix.
 *
 * Any further use of the handle is undefined after this function is called.
 *
 * \param matrix The matrix handle.
 */
void dyn_matrix_destroy(struct dyn_matrix* matrix);

/**
 * \brief Transpose a dynamic matrix.
 *
 * \note Currently only works if the matrix is square.
 *
 * \param matrix The matrix handle.
 *
 * \return \ref status_t
 */
status_t dyn_matrix_transpose(struct dyn_matrix* matrix);

/**
 * \brief Print a dynamic matrix.
 *
 * Calls the printe() function on each element in the matrix.
 *
 * \param matrix The matrix handle.
 */
void dyn_matrix_print(const struct dyn_matrix* matrix);

/**
 * \brief Resize a dynamic matrix manually.
 *
 * \param matrix The matrix handle.
 * \param u The desired # of rows. Can be <,>,= the current #. If =, no action
 *          is performed.
 *
 * \param v The desired # of columns. Can be <,>,= the current #. If =, no
 *          action is performed.
 *
 * \return \ref status_t
 */
status_t dyn_matrix_resize(struct dyn_matrix* matrix, size_t u, size_t v);
/**
 * \brief Set an element in the dynamic matrix to a specific value.
 *
 * If the row/column is outside the current bounds of the matrix, the matrix
 * will be resized to accommodate.
 *
 * \param matrix The matrix handle.
 * \param u Element row index.
 * \param v Element column index.
 * \param w New element value.
 *
 * \return \ref status_t
 */
status_t dyn_matrix_set(struct dyn_matrix* matrix,
                        size_t u,
                        size_t v,
                        const void *w);

END_C_DECLS
