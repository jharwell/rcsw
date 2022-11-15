/**
 * \file dynamic_matrix.h
 * \ingroup ds
 * \brief Implementation of dynamic matrix.
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
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * This file is part of RCSW.
 *
 * RCSW is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * RCSW is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * RCSW.  If not, see <http://www.gnu.org/licenses/
 */

#ifndef INCLUDE_RCSW_DS_DYNAMIC_MATRIX_H_
#define INCLUDE_RCSW_DS_DYNAMIC_MATRIX_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/darray.h"
#include "rcsw/ds/llist.h"

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/
/**
 * \brief Representation of a dynamically-sized matrix using a dynamic array row
 * vectors, with each row vector also being a dynamic array.
 */
struct dynamic_matrix {
  struct darray* rows;  /// Vector of row vectors.
  size_t n_rows;        /// # of rows in matrix.
  size_t n_cols;        /// # of columns in matrix.
  uint32_t flags;       /// Run-time configuration flags.
  size_t el_size;       /// Size of matrix elements in bytes.

  /** For printing an element. Can be NULL. */
  void (*printe)(const void *const e);
};

/*******************************************************************************
 * Inline Functions
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Access an element within a dynamic matrix. The element must exist
 * within the matrix (i.e. this function does not expand the matrix).
 *
 * \param matrix The matrix handle.
 * \param u The row within the matrix.
 * \param v The column within the matrix.
 *
 * \return Reference to element, or NULL if an error occurred.
 */
static inline void* dynamic_matrix_access(const struct dynamic_matrix* const matrix,
                                          size_t u, size_t v) {
    RCSW_FPC_NV(NULL, NULL != matrix, u < matrix->n_rows,
              v < matrix->n_cols);
    return darray_data_get((struct darray*)darray_data_get(matrix->rows, u), v);
}

/**
 * \brief Calculate the # of bytes required for the initial size of the dynamic
 * matrix.
 *
 * \param n_rows Initial # rows.
 * \param n_cols Initial # columns.
 * \param el_size Size of the elements in bytes.
 *
 * \return The # of bytes required.
 */
static inline size_t dynamic_matrix_space(size_t n_rows, size_t n_cols,
                                         size_t el_size) {
  return darray_element_space(n_cols, el_size) * n_rows +
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
static inline status_t dynamic_matrix_clear(struct dynamic_matrix* const matrix,
                                           size_t u, size_t v) {
    RCSW_FPC_NV(ERROR, NULL != matrix, u < matrix->n_rows, v < matrix->n_cols);
    ds_elt_clear(dynamic_matrix_access(matrix, u, v), matrix->el_size);
    return OK;
}

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
/**
 * \brief Initialize a dynamic matrix.
 *
 * \param matrix_in An application allocated handle for the dynamic matrix. Can
 * be NULL, depending on if \ref DS_APP_DOMAIN_HANDLE is passed or not.
 * \param params The initialization parameters.
 *
 * \return The initialized matrix, or NULL if an error occurred.
 */
struct dynamic_matrix* dynamic_matrix_init(struct dynamic_matrix* matrix_in,
                                           const struct ds_params* params) RCSW_CHECK_RET;

/**
 * \brief Destroy a dynamic matrix. Any further use of the handle is undefined
 * after this function is called.
 *
 * \param matrix The matrix handle.
 */
void dynamic_matrix_destroy(struct dynamic_matrix* matrix);

/**
 * \brief Transpose a dynamic matrix. Currently only works if the matrix is
 * square.
 *
 * \param matrix The matrix handle.
 *
 * \return \ref status_t
 */
status_t dynamic_matrix_transpose(struct dynamic_matrix* matrix);

/**
 * \brief Print a dynamic matrix, by calling the printe() function on each
 * element in the matrix.
 *
 * \param matrix The matrix handle.
 */
void dynamic_matrix_print(const struct dynamic_matrix* matrix);

/**
 * \brief Resize a dynamic matrix manually.
 *
 * \param matrix The matrix handle.
 * \param u The desired # of rows (can be less than the current #, in which case
 *          no expansion of rows is performed).
 * \param v The desired # of columns (can be less than the current #, in which
 * case no expansion of columns in performed).
 *
 * \return \ref status_t
 */
status_t dynamic_matrix_resize(struct dynamic_matrix* matrix, size_t u,
                               size_t v);
/**
 * \brief Set an element in the dynamic matrix to a specific value. If the
 * row/column is outside the current bounds of the matrix, the matrix will be
 * resized to accommodate.
 *
 * \param matrix The matrix handle.
 * \param u Element row index.
 * \param v Element column index.
 * \param w New element value.
 *
 * \return \ref status_t
 */
status_t dynamic_matrix_set(struct dynamic_matrix* matrix, size_t u,
                            size_t v, const void *w);

END_C_DECLS

#endif /* INCLUDE_RCSW_DS_DYNAMIC_MATRIX_H_ */
