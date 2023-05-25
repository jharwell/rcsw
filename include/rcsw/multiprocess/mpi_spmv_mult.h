/**
 * \file mpi_spmv_mult.h
 * \ingroup multiprocess
 * \brief Implementation of MPI-based sparse matrix-vector multiplication.
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <mpi.h>
#include "rcsw/ds/csmatrix.h"

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/
/**
 * Parameters for sparse matrix/vector multiplication via MPI
 */
struct mpi_spmv_mult_params {
    int mpi_rank;
    int mpi_world_size;

    /** The sparse matrix to multiply with (only significant at master) */
    struct csmatrix *matrix;
};

/**
 * Sparse matrix -> vector multiplier.
 */
struct mpi_spmv_mult {
    struct csmatrix *matrix;    /// The sparse matrix
    struct darray *vector_in;   /// The vector to multiply with
    struct darray *vector_out;  /// The resulting vector

    /**
     * An array containing the # of elements allocated to each rank (at the
     * root) in terms of matrix entries, and NULL at all other ranks.
     */
    int* rank_alloc_elts;

    /**
     * An array containing the # rows assigned to each rank (at the root), and
     * NULL at all other ranks.
     */
    int* rank_alloc_rows;

    /**
     * Array of what mpi ranks are responsible for the entries in a particular
     * row of the matrix. All ranks have a copy.
     */
    int* row_owners;

    /**
     * Prefix sums for # of elements allocated to each rank over the rows, used
     * for collecting the results of the multiplication at the root.
     */
    int* rank_alloc_row_prefix_sums;

    /**
     * Prefix sums for # of elements allocated to each rank over the columns,
     * used for correct sending/receiving of matrix data during execution.
     */
    int* rank_alloc_col_prefix_sums;

    /**
     * Sizes of each row in the matrix, used for collecting the results of the
     * multiplication at the root.
     */
    int* row_sizes;

    int mpi_rank;
    int mpi_world_size;

    /**
     * The total # of rows in the matrix broadcast during
     * initialization. Needed so that the row membership array can be
     * initialized to the proper size.
     */
    int n_rows_init;

    /**
     * The total # of rows in the matrix broadcast during
     * initialization.
     */
    int n_cols_init;

    /**
     * The # of rows allocated to a particular MPI rank
     */
    int n_rows_alloc;

    /**
     * The # of elements allocated to a particular MPI rank
     */
    int n_elts_alloc;
    int row_alloc_start;  /// Starting row allocated to a rank.

    /**
     * The # of elements allocated to a particular MPI rank, but taken over the
     * columns of the ORIGINAL matrix, rather than the submatrix currently
     * assigned to the rank.
     *
     */
    int n_elts_alloc_init_transpose;

    /**
     * A custom datatype for sending/receiving non-zero parts of the vector to
     * multiply with. It was much easier to creating a mapping datatype than to
     * try to finagle it with arrays.
     */
    MPI_Datatype spmv_comm_type;
};

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Initialize the MPI multiplier. All ranks call this function.
 *
 * \param params The initialization parameters.
 *
 * \return The initialized multiplier, or NULL if an ERROR occurred.
 */
struct mpi_spmv_mult* mpi_spmv_mult_init(
    const struct mpi_spmv_mult_params* const params);

/**
 * \brief Destroy an MPI multiplier.
 *
 * Any further use of the handle after calling this function is undefined.
 *
 * \param mult The multiplier handle.
 */
void mpi_spmv_mult_destroy(struct mpi_spmv_mult* const mult);

/**
 * \brief Initialize multiplier data structures on all non-zero ranks by
 * distributing the dimensions of the matrix/vector components each rank will be
 * responsible for.
 *
 * This must be called after \ref mpi_spmv_mult_init(), but before \ref
 * mpi_spmv_mult_distribute() or \ref mpi_spmv_mult_exec().
 *
 * \param mult The multiplier handle.
 */
status_t mpi_spmv_mult_ds_init(struct mpi_spmv_mult* const mult);

/**
 * \brief Distribute the data in the matrix/vector at the root to all ranks
 *
 * This must be called after \ref mpi_spmv_mult_ds_init(), but before \ref
 * mpi_spmv_mult_exec().
 *
 * \param mult The multiplier handle.
 * \param vector The vector to multiply with
 */
status_t mpi_spmv_mult_distribute(struct mpi_spmv_mult* const mult,
                                  struct darray* vector);

/**
 * \brief Execute the multiply in parallel via MIA.
 *
 * This must be called after \ref mpi_spmv_mult_distribute(),
 *
 * \param mult The multiplier handle.
 */
struct darray* mpi_spmv_mult_exec(struct mpi_spmv_mult* const mult);

END_C_DECLS

