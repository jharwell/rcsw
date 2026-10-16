/**
 * \file
 *
 * \copyright 2017 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * \ingroup ds
 *
 * \brief Fixed-size matrix.
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/al/types.h"
#include "rcsw/core/compilers.h"
#include "rcsw/core/core.h"
#include "rcsw/core/fpc.h"
#include "rcsw/ds/ds.h"

/*******************************************************************************
 * Types
 ******************************************************************************/
/**
 * \brief Static matrix initialization parameters.
 */
struct matrix_config {
  /**
   * For printing an element. Can be NULL. If NULL, \ref matrix_print() is
   * disabled.
   */
  void (*printe)(const void* e);

  /**
   * Pointer to application-allocated space for storing the data managed by the
   * \ref matrix. Ignored unless \ref RCSW_NOALLOC_DATA is passed.
   */
  dptr_t* elements;

  /**
   * Size of elements in bytes.
   */
  size_t elt_size;

  /**
   * Run-time configuration flags. Valid flags are:
   *
   * - \ref RCSW_ZALLOC
   * - \ref RCSW_NOALLOC_HANDLE
   * - \ref RCSW_NOALLOC_DATA
   *
   * All other flags ignored.
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
 *
 * \note \ref matrix_transpose() supports square matrices only.
 */
struct matrix {
  /**
   * The configuration the matrix was initialized with. \ref
   * matrix_config.elements is as passed (NULL if the library allocated the
   * storage); use \ref matrix.elements for the storage in use.
   */
  struct matrix_config cfg;

  /** Matrix data. */
  dptr_t* elements;
};

/*******************************************************************************
 * Public API
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
                                  size_t                     u,
                                  size_t                     v) {
  RCSW_FPC_NV(NULL,
              NULL != matrix,
              u < matrix->cfg.n_rows,
              v < matrix->cfg.n_cols);
  return (uint8_t*)matrix->elements +
         (matrix->cfg.n_cols * matrix->cfg.elt_size * u) +
         (matrix->cfg.elt_size * v);
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
                                        size_t               u,
                                        size_t               v) {
  RCSW_FPC_NV(ERROR,
              NULL != matrix,
              u < matrix->cfg.n_rows,
              v < matrix->cfg.n_cols);
  ds_elt_clear(matrix_access(matrix, u, v), matrix->cfg.elt_size);
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
                                  size_t               u,
                                  size_t               v,
                                  const void* const    w) {
  RCSW_FPC_NV(ERROR,
              NULL != matrix,
              NULL != w,
              u < matrix->cfg.n_rows,
              v < matrix->cfg.n_cols);

  ds_elt_copy(matrix_access(matrix, u, v), w, matrix->cfg.elt_size);
  return OK;
} /* matrix_set() */

/**
 * \brief Return if the \ref matrix is square or not.
 */
static inline bool_t matrix_issquare(const struct matrix* const matrix) {
  RCSW_FPC_NV(false, NULL != matrix);
  return matrix->cfg.n_cols == matrix->cfg.n_rows;
}

/**
 * \brief Initialize a static matrix.
 *
 * \param matrix_in Caller storage for the handle, used only if \ref
 *                  RCSW_NOALLOC_HANDLE is passed; ignored (may be NULL)
 *                  otherwise. See \rcswdoc{concepts/memory-model}.
 *
 * \param params The initialization parameters.
 *
 * \return The initialized matrix, or NULL if an error occurred.
 */
RCSW_API struct matrix* matrix_init(struct matrix*              matrix_in,
                                    const struct matrix_config* params);

/**
 * \brief Destroy a static matrix.
 *
 * Any further use of the handle is undefined after this function is called.
 *
 * \param matrix The matrix handle.
 */
RCSW_API void matrix_destroy(struct matrix* matrix);

/**
 * \brief Transpose a static matrix in place.
 *
 * \note Square matrices only. Transposing a non-square matrix in place would
 * also have to swap \ref matrix_config.n_rows and \ref matrix_config.n_cols
 * and permute the storage by cycles, which is not implemented. A non-square
 * matrix fails the function precondition (ERROR under \c LIBRA_FPC_RETURN). To
 * transpose an M x N matrix, copy it element by element into an N x M one.
 *
 * \param matrix The matrix handle.
 *
 * \return \ref status_t
 */
RCSW_API status_t matrix_transpose(struct matrix* matrix);

/**
 * \brief Print a static matrix
 *
 * Calls the printe() function on each element in the matrix.
 *
 * \param matrix The matrix handle.
 */
RCSW_API void matrix_print(const struct matrix* matrix);

END_C_DECLS
