/**
 * \file csmatrix.h
 * \ingroup ds
 * \brief Compressed sparse matrix implementation in row major order.
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
 * Constant Definitions
 ******************************************************************************/
/**
 * The valid types of data that the sparse matrix will hold.
 */
enum csmatrix_type {
    ekCSMATRIX_INT,
    ekCSMATRIX_FLOAT,
    ekCSMATRIX_DOUBLE
};

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/
/**
 * \brief Sparse matrix initialization parameters.
 */
struct csmatrix_params {
  /**
   * Initial number of rows in the matrix.
   */
  size_t n_rows;

  /**
   * Initial number of columns in the matrix
   */
  size_t n_cols;

  /**
   * Number of non-zero entries in the matrix.
   */
  size_t n_nz_elts;

  /**
   * Configuration flags. See \ref csmatrix.flags for valid flags.
   */
  uint32_t flags;

  /**
   * What type of numbers the matrix will contain
   */
  enum csmatrix_type type;
};

/**
 * \brief Representation of a sparse matrix in a compressed, row-major format.
 *
 * It should be noted that once initialized, the # rows/columns cannot be
 * changed.  Yes, I know that there are other (probably more optimized)
 * implementations out there, but sometimes you cannot use them (platform,
 * licensing, etc.), hence this module. It assumes row-major ordering.
 */
struct csmatrix {
  /**
   * Holds the row indices of the non-zero entries in the matrix.
   */
  struct darray inner_indices;

  /**
   * Holds the index of first nonzero in the inner_indices array for each row.
   */
  struct darray outer_starts;

  /**
   * Holds the coefficient values of the non-zeros.
   */
  struct darray values;

  /**
   * Initial number of rows in the matrix.
   */
  size_t n_rows;

  /**
   * Initial number of columns in the matrix
   */
  size_t n_cols;

  /**
   * Number of non-zero entries in the matrix.
   */
  size_t n_nz_elts;

  /**
   * Configuration flags. See \ref csmatrix.flags for valid flags.
   */
  uint32_t flags;

  /**
   * What type of numbers the matrix will contain
   */
  enum csmatrix_type type;

  /**
   * Array of linked lists, one per column, containing the row indices that
   * for that column. Necessary for transposes so things don't take FOREVER.
   */
  struct llist *cols;


  /**
   * Space for link list nodes (use one contiguous block to improve cache
   * performance)
   */
  uint8_t * nodes;

  /**
   * Space for link list elements (use one contiguous block to improve cache
   * performance)
   */
  uint8_t * elts;

  int* csizes;
};

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Get the size of elements stored in the matrix.
 *
 * \param matrix The matrix handle.
 *
 * \return The size of elements in bytes.
 */
static inline size_t csmatrix_type_size(const struct csmatrix* const matrix) {
  RCSW_FPC_NV(0, NULL != matrix);

  switch (matrix->type) {
    case ekCSMATRIX_INT:
        return sizeof(int);
        break;
    case ekCSMATRIX_FLOAT:
        return sizeof(float);
        break;
    case ekCSMATRIX_DOUBLE:
        return sizeof(double);
        break;
    default:
      break;
    } /* switch() */
  return 0;
} /* csmatrix_type_size() */

/**
 * \brief Get # of rows in the matrix. This is the same for both sparse and
 * dense matrices.
 *
 * \param matrix The matrix handle.
 *
 * \return The # of rows.
 */
static inline size_t csmatrix_n_rows(
    const struct csmatrix* const matrix) {
    RCSW_FPC_NV(0, NULL != matrix);

    /*
     * Last element does not correspond to a row--just there to make math
     * simple
     */
    return darray_size(&matrix->outer_starts) - 1;
} /* csmatrix_n_rows() */

/**
 * \brief Get the # of effective columns in the matrix.
 *
 * This is equivalent to the # of unique indices in the inner indices array.
 *
 * \param matrix The matrix handle.
 *
 * \return The # of effective columns.
 */
static inline size_t csmatrix_n_eff_cols(const struct csmatrix* const matrix) {
    RCSW_FPC_NV(0, NULL != matrix);
    return matrix->n_eff_cols;
} /* csmatrix_n_eff_cols() */

/**
 * \brief Get the # of total columns in the matrix, if it were represented in an
 * uncompressed form.
 *
 * This is never modified after initialization, and serves as a convenience
 * method when attempted to print a matrix.
 *
 * \param matrix The matrix handle.
 *
 * \return The total # of columns.
 */
static inline size_t csmatrix_n_cols(const struct csmatrix* const matrix) {
    RCSW_FPC_NV(0, NULL != matrix);
    return matrix->n_cols;
} /* csmatrix_n_eff_cols() */

/**
 * \brief Get the current # elements in a \ref csmatrix,
 */
static inline size_t csmatrix_size(
    const struct csmatrix* const matrix) {
    RCSW_FPC_NV(0, NULL != matrix);
    return (size_t)*(int*)darray_data_get(&matrix->outer_starts,
                                          darray_size(&matrix->outer_starts)-1);
} /* csmatrix_size() */

/**
 * \brief Get a row in \ref matrix.
 */
static inline int* csmatrix_row(const struct csmatrix* const matrix,
                                size_t row) {
    RCSW_FPC_NV(0, NULL != matrix);
    return darray_data_get(&matrix->inner_indices,
                           (size_t)*(int*)darray_data_get(&matrix->outer_starts, row));
} /* csmatrix_row() */


/**
 * \brief Get the # of non-zero entries in a row.
 *
 * \param matrix The matrix handle.
 * \param row The row
 *
 * \return The # of non-zero entries
 */
static inline size_t csmatrix_rsize(
    const struct csmatrix* const matrix, size_t row) {
    RCSW_FPC_NV(0, NULL != matrix, row < darray_size(&matrix->outer_starts));
    size_t row_start = (size_t)*(int*)darray_data_get(&matrix->outer_starts,
                                                      row);
    size_t row_end = (size_t)*(int*)darray_data_get(&matrix->outer_starts,
                                                    row+1);

    return row_end - row_start;
} /* csmatrix_rsize() */

/**
 * \brief Get the # of non-zero entries in a column.
 *
 * \param matrix The matrix handle.
 * \param col The column
 *
 * \return The # of non-zero entries
 */
static inline size_t csmatrix_csize(const struct csmatrix* const matrix,
                                    size_t col) {
    RCSW_FPC_NV(0, NULL != matrix);
    return (size_t)matrix->csizes[col];
} /* csmatrix_csize() */

/**
 * \brief Direct access to column sizes array
 *
 * \param matrix The matrix handle.
 *
 * \return Pointer to first element in csizes array
 */
static inline int* csmatrix_csizes(const struct csmatrix* const matrix) {
    RCSW_FPC_NV(0, NULL != matrix);
    return matrix->csizes;
} /* csmatrix_csize() */

/**
 * \brief Direct access to inner indices array.
 *
 * \param matrix The matrix handle.
 *
 * \return Pointer to the first element in the inner indices array.
 */
static inline int* csmatrix_inner_indices(
    const struct csmatrix* const matrix) {
    RCSW_FPC_NV(0, NULL != matrix);
    return darray_data_get(&matrix->inner_indices, 0);
} /* csmatrix_rsize() */

/**
 * \brief Direct access to outer starts array.
 *
 * \param matrix The matrix handle.
 *
 * \return Pointer to the first element in the outer starts array.
 */
static inline int* csmatrix_outer_starts(
    const struct csmatrix* const matrix) {
    RCSW_FPC_NV(0, NULL != matrix);
    return darray_data_get(&matrix->outer_starts, 0);
} /* csmatrix_rsize() */

/**
 * \brief Direct access to values array.
 *
 * \param matrix The matrix handle.
 *
 * \return Pointer to the first element in the values array.
 */
static inline double* csmatrix_values(
    const struct csmatrix* const matrix) {
    RCSW_FPC_NV(0, NULL != matrix);
    return darray_data_get(&matrix->values, 0);
} /* csmatrix_rsize() */

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
/**
 * \brief Initialize a compressed, sparse matrix. It should be noted that once
 * initialized, the row/column dimensions of the matrix cannot be modified.
 *
 * \param matrix_in The matrix handle. Ignored unless \ref RCSW_NOALLOC_DATA
 *                  is passed, in which it is used instead of the matrix
 *                  allocated memory for its own handle.
 *
 * \param params The initialization parameters
 *
 * \return The initialized matrix, or NULL if an error occurred.
 */
struct csmatrix* csmatrix_init(struct csmatrix* matrix_in,
                               const struct csmatrix_params* params);

/**
 * \brief Destroy a csmatrix.
 *
 * Any further use of the handle after calling this function is undefined.
 *
 * \param matrix The matrix handle.
 */
void csmatrix_destroy(struct csmatrix* matrix);

/**
 * \brief Add an entry to the matrix.
 *
 * The (row, col) cannot be outside of the current dimensions of the
 * matrix.
 *
 * \param matrix The matrix handle.
 * \param sequential_insertions If true, the application promises to insert an
 * entire row sequentially. This greatly speeds up the process of insertion.
 * \param row The row for the new entry.
 * \param col The column for the new entry.
 * \param e The new entry.
 *
 * \return \ref status_t.
 */
status_t csmatrix_entry_add(struct csmatrix* matrix,
                            bool_t sequential_insertions, size_t row,
                            size_t col, const void* e);
/**
 * \brief Remove an entry to the matrix.
 *
 * The (row, col) cannot be outside of the current dimensions of the
 * matrix.
 *
 * \param matrix The matrix handle.
 * \param row The row for the entry to delete.
 * \param col The column for the entry to delete.
 *
 * \return \ref status_t.
 */
status_t csmatrix_entry_delete(struct csmatrix* matrix,
                               size_t row, size_t col);

/**
 * \brief Set a currently existing entry in the matrix to a new value.
 *
 * The (row, col) cannot be outside of the current dimensions of the
 * matrix.
 *
 * \param matrix The matrix handle.
 * \param row The row for the entry to set.
 * \param col The column for the entry to set.
 * \param e The new entry.
 *
 * \return \ref status_t.
 */
status_t csmatrix_entry_set(struct csmatrix* matrix,
                            size_t row,
                            size_t col,
                            const void* e);

/**
 * \brief Get the index of a specific (row, col) within the inner index array.
 *
 * \param matrix The matrix handle.
 * \param row The row for the entry
 * \param col The column for the entry.
 *
 * \return The index, or -1 if an ERROR occurred or the (row, col) wasn't
 * found.
 */
int csmatrix_inner_index_get(const struct csmatrix* matrix,
                             size_t row, size_t col);

/**
 * \brief Get a reference to a specific (row, col) within the matrix.
 *
 * \param matrix The matrix handle.
 * \param row The row for the entry to delete.
 * \param col The column for the entry to delete.
 *
 * \return The entry, or NULL if an ERROR occurred or the (row, col) wasn't
 * found.
 */
void* csmatrix_entry_get(const struct csmatrix* matrix,
                         size_t row, size_t col);

/**
 * \brief Normalize across the columns of the matrix.
 *
 * This is useful if you need a column stochastic matrix.
 *
 * \param matrix The matrix handle.
 *
 * \return \ref status_t.
 */
status_t csmatrix_cols_normalize(struct csmatrix* matrix);

/**
 * \brief Multiply a sparse matrix with a vector.
 *
 * The vector must also be compressed (i.e. you can do a dot product of the
 * vector and the array storing the non-zero values in the matrix). The type of
 * data stored in the vectors must be the same as that stored in the matrix.
 *
 * \param matrix The matrix handle.
 * \param vector_in The vector to multiply with.
 * \param vector_out The resulting vector. Must have same capacity as input
 * vector.
 *
 * \return \ref status_t.
 */
status_t csmatrix_vmult(const struct csmatrix* matrix,
                        const struct darray* vector_in,
                        struct darray* vector_out);

/**
 * \brief Transpose a matrix, returning a new matrix.
 *
 * The original matrix is not modified.
 *
 * \param matrix The matrix handle.
 *
 * \return The transposed matrix, or NULL if an error occurred.
 */
struct csmatrix* csmatrix_transpose(struct csmatrix* matrix);

/**
 * \brief Resize a matrix to the specified row x col.
 *
 * All elements beyond the specified dimension are removed. This has the side
 * effect of clearing the column sizes and column lists.
 *
 * \param matrix The matrix handle.
 * \param n_rows The new # of rows.
 * \param n_nz_elts The new # of columns.
 *
 * \return \ref status_t.
 */
status_t csmatrix_resize(struct csmatrix* matrix, size_t n_rows,
                         size_t n_nz_elts);

/**
 * \brief Calculate the column lists for a matrix
 *
 * The happens automatically when using \ref csmatrix_entry_add(), but if you
 * manually copy data into/out of the matrix, and want to adjust, this function
 * can be used.
 *
 * \param matrix The matrix handle.
 *
 * \return \ref status_t.
 */
status_t csmatrix_calc_clists(struct csmatrix* matrix);

/**
 * \brief Print a csmatrix to stdout.
 *
 * \param matrix The matrix handle.
 */
void csmatrix_print(const struct csmatrix* matrix);

END_C_DECLS
