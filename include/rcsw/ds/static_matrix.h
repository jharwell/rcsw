/**
 * \file static_matrix.h
 * \ingroup ds
 * \brief Implementation of static matrix.
 *
 * Static in the sense that the matrix dimensions cannot be changed after
 * initialization. Assumes row-major order (this is C after all).
 *
 * Should only be used for applications where the matrix is dense, or when the
 * dimensions of the matrix are small (if the dimensions are large AND the
 * matrix is sparse, it will definitely be really inefficient, and possibly
 * won't even fit into memory).
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef INCLUDE_RCSW_DS_STATIC_MATRIX_H_
#define INCLUDE_RCSW_DS_STATIC_MATRIX_H_

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
 * \brief Representation of a fixed-size (static) matrix using a single
 * contiguous array and indexing said array in 2 dimensions.
 */
struct static_matrix {
  size_t n_rows;      /// # of rows in matrix.
  size_t n_cols;      /// # of columns in matrix.
  uint8_t* elements;  /// Matrix data.
  uint32_t flags;     /// Run-time configuration flags.
  size_t el_size;     /// Size of matrix elements in bytes.
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
static inline void* static_matrix_access(const struct static_matrix* const matrix,
                                         size_t u, size_t v) {
  RCSW_FPC_NV(NULL, NULL != matrix, u < matrix->n_rows,
            v < matrix->n_cols);
  return matrix->elements + (matrix->n_cols * matrix->el_size * u) +
      (matrix->el_size * v);
}

/**
 * \brief Calculate the # of bytes required for the static matrix.
 *
 * \param n_rows Initial # rows.
 * \param n_cols Initial # columns.
 * \param el_size Size of the elements in bytes.
 *
 * \return The # of bytes required.
 */
static inline size_t static_matrix_space(size_t n_rows, size_t n_cols,
                                         size_t el_size) {
  return ds_calc_element_space1(n_rows * n_cols, el_size);
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
static inline status_t static_matrix_clear(struct static_matrix* const matrix,
                                           size_t u, size_t v) {
  RCSW_FPC_NV(ERROR, NULL != matrix, u < matrix->n_rows, v < matrix->n_cols);
  ds_elt_clear(static_matrix_access(matrix, u, v), matrix->el_size);
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
static inline status_t static_matrix_set(struct static_matrix* const matrix,
                                         size_t u, size_t v,
                                         const void *const w) {
  RCSW_FPC_NV(ERROR, NULL != matrix, NULL != w, u < matrix->n_rows,
            v < matrix->n_cols);

  ds_elt_copy(static_matrix_access(matrix, u, v), w, matrix->el_size);
  return OK;
} /* static_matrix_set() */

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
/**
 * \brief Initialize a static matrix.
 *
 * \param matrix_in An application allocated handle for the static matrix. Can
 * be NULL, depending on if \ref DS_APP_DOMAIN_HANDLE is passed or not.
 * \param params The initialization parameters.
 *
 * \return The initialized matrix, or NULL if an error occurred.
 */
struct static_matrix* static_matrix_init(struct static_matrix* matrix_in,
                                         const struct ds_params* params);

/**
 * \brief Destroy a static matrix. Any further use of the handle is undefined
 * after this function is called.
 *
 * \param matrix The matrix handle.
 */
void static_matrix_destroy(struct static_matrix* matrix);

/**
 * \brief Transpose a static matrix. Currently only works if the matrix is
 * square.
 *
 * \param matrix The matrix handle.
 *
 * \return \ref status_t
 */
status_t static_matrix_transpose(struct static_matrix* matrix);

/**
 * \brief Print a static matrix, by calling the printe() function on each
 * element in the matrix.
 *
 * \param matrix The matrix handle.
 */
void static_matrix_print(const struct static_matrix* matrix);

END_C_DECLS

#endif /* INCLUDE_RCSW_DS_STATIC_MATRIX_H_ */
