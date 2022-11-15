/**
 * \file mpi_radix_sort.h
 * \ingroup multiprocess
 * \brief Multiprocess radix sorter using MPI.
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

#ifndef INCLUDE_RCSW_MULTIPROCESS_MPI_RADIX_SORT_H_
#define INCLUDE_RCSW_MULTIPROCESS_MPI_RADIX_SORT_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/common/common.h"

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/
/**
 * \brief Parameters for \ref mpi_radix_sorter.
 */
struct mpi_radix_sorter_params {
    size_t* data;        /// Array to sort.
    size_t n_elts;       /// # elements to sort.
    size_t base;         /// base for sort (10, 16, etc.).
    int mpi_rank;        /// Rank in MPI_COMM_WORLD .
    int mpi_world_size;  /// Total # processes.
};

/**
 * \brief Handle for MPI implementation of radix sort.
 */
struct mpi_radix_sorter {
    size_t* data;       /// The data a rank is working on.
    size_t* cum_data;   /// The total data to be sorted. Only valid at root.
    size_t n_elts;      /// Total # elements. Valid at all ranks */
    size_t base;        /// base for sort (10, 16, etc.).

    size_t chunk_size;  /// Size of each worker chunk. Valid at all ranks.
    int mpi_rank;
    int mpi_world_size;
    size_t* tmp_arr;    /// tmp array used for doing the actual sort.

    /**
     * Prefix sums for each rank for each symbol in a digit.
     */
    size_t *prefix_sums;

    /**
     * Cumulative prefix sums used by the master to compute receive and
     * displacement counts.
     */
    size_t *cum_prefix_sums;
};

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Initialize a multi-process radix sorter.
 *
 * \param params Initialization parameters.
 *
 * \return The initialized sorter, or NULL if an error occurred.
 */
struct mpi_radix_sorter* mpi_radix_sorter_init(
    const struct mpi_radix_sorter_params* const params) RCSW_CHECK_RET;

/**
 * \brief Deallocate/destroy a sorter after use.
 *
 * \param sorter The sorter to destroy.
 */
void mpi_radix_sorter_destroy(struct mpi_radix_sorter* const sorter);

/**
 * \brief Perform radix sort in parallel using MPI.
 *
 * \param sorter The initialized sorter.
 *
 * \return \ref status_t.
 */
status_t mpi_radix_sorter_exec(struct mpi_radix_sorter* const sorter);

END_C_DECLS

#endif /* INCLUDE_RCSW_MULTIPROCESS_MPI_RADIX_SORT_H_ */
