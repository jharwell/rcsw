/**
 * \file
 *
 * \copyright 2017 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * \ingroup ds
 *
 * \brief Dynamically sized matrix.
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/al/types.h"
#include "rcsw/core/compilers.h"
#include "rcsw/core/core.h"
#include "rcsw/core/fpc.h"
#include "rcsw/ds/darray.h"
#include "rcsw/ds/ds.h"

/*******************************************************************************
 * Types
 ******************************************************************************/
/**
 * \brief Dynamic matrix initialization parameters.
 */
struct dynmatrix_config {
  /**
   * For printing an element. Can be NULL.
   */
  void (*printe)(const void* e);

  /**
   * Size of elements in bytes.
   */
  size_t elt_size;

  /**
   * Configuration flags. See \ref dynmatrix.flags for valid flags; in
   * particular, element storage is always heap-allocated.
   */
  uint32_t flags;

  size_t n_rows;  ///< # rows in matrix.
  size_t n_cols;  ///< # columns in matrix.
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
 * Besides \ref dynmatrix_resize(), the matrix grows when \ref dynmatrix_set()
 * is given an out-of-bounds index; no other function resizes it.
 *
 * \note Only the handle can be caller-provided (\ref RCSW_NOALLOC_HANDLE). The
 * row vector and every row are always heap-allocated, so \ref
 * RCSW_NOALLOC_DATA and \ref RCSW_NOALLOC_META have no effect and a dynmatrix
 * cannot be used in builds without a heap. Use \ref matrix (fixed
 * dimensions, caller-provided storage allowed) there instead.
 */
struct dynmatrix {
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
   * - \ref RCSW_ZALLOC (zeroes the handle; elements are always
   *   zero-initialized regardless)
   *
   * All other flags are ignored, including \ref RCSW_NOALLOC_DATA and \ref
   * RCSW_NOALLOC_META.
   */
  uint32_t flags;

  /**
   * For printing an element. Can be NULL. If NULL, then \ref
   * dynmatrix_print() is disabled.
   */
  void (*printe)(const void* const e);
};

/*******************************************************************************
 * Public API
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
static inline void* dynmatrix_access(const struct dynmatrix* const matrix,
                                     size_t                        u,
                                     size_t                        v) {
  RCSW_FPC_NV(NULL, NULL != matrix, u < matrix->n_rows, v < matrix->n_cols);
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
static inline size_t dynmatrix_space(size_t n_rows,
                                     size_t n_cols,
                                     size_t elt_size) {
  return (darray_element_space(n_cols, elt_size) * n_rows) +
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
static inline status_t dynmatrix_clear(struct dynmatrix* const matrix,
                                       size_t                  u,
                                       size_t                  v) {
  RCSW_FPC_NV(ERROR, NULL != matrix, u < matrix->n_rows, v < matrix->n_cols);
  ds_elt_clear(dynmatrix_access(matrix, u, v), matrix->elt_size);
  return OK;
}

/**
 * \brief Return if the \ref dynmatrix is square or not.
 */
static inline bool_t dynmatrix_issquare(const struct dynmatrix* const matrix) {
  RCSW_FPC_NV(false, NULL != matrix);

  return matrix->n_cols == matrix->n_rows;
}

/**
 * \brief Initialize a dynamic matrix.
 *
 * \param matrix_in Caller storage for the handle, used only if \ref
 *                  RCSW_NOALLOC_HANDLE is passed; ignored (may be NULL)
 *                  otherwise. See \rcswdoc{concepts/memory-model}.
 *
 * \param params The initialization parameters.
 *
 * \return The initialized matrix, or NULL if an error occurred.
 */
RCSW_API struct dynmatrix* dynmatrix_init(
  struct dynmatrix* matrix_in, const struct dynmatrix_config* params) RCSW_WUR;

/**
 * \brief Destroy a dynamic matrix.
 *
 * Any further use of the handle is undefined after this function is called.
 *
 * \param matrix The matrix handle.
 */
RCSW_API void dynmatrix_destroy(struct dynmatrix* matrix);

/**
 * \brief Transpose a dynamic matrix in place.
 *
 * \note Square matrices only; a non-square matrix fails the function
 * precondition (ERROR under \c LIBRA_FPC_RETURN). For an M x N matrix, \ref
 * dynmatrix_resize() it to K x K with K = max(M,N), transpose, then resize it
 * to N x M.
 *
 * \param matrix The matrix handle.
 *
 * \return \ref status_t
 */
RCSW_API status_t dynmatrix_transpose(struct dynmatrix* matrix);

/**
 * \brief Print a dynamic matrix.
 *
 * Calls the printe() function on each element in the matrix.
 *
 * \param matrix The matrix handle.
 */
RCSW_API void dynmatrix_print(const struct dynmatrix* matrix);

/**
 * \brief Resize a dynamic matrix manually.
 *
 * Existing elements inside the new bounds keep their values; new elements are
 * zero. The resize is all-or-nothing: if it fails (e.g., out of memory), the
 * matrix keeps its old dimensions and contents, though some memory reserved
 * along the way may not be released until a later resize or destroy.
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
RCSW_API status_t dynmatrix_resize(struct dynmatrix* matrix, size_t u, size_t v);

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
RCSW_API status_t dynmatrix_set(struct dynmatrix* matrix,
                                size_t            u,
                                size_t            v,
                                const void*       w);

END_C_DECLS
