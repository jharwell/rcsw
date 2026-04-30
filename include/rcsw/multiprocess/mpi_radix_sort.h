/**
 * \file
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * \ingroup multiprocess
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stddef.h>

#include "rcsw/core/compilers.h"
#include "rcsw/core/core.h"

/*******************************************************************************
 * Types
 ******************************************************************************/
/**
 * \brief Parameters for \ref mpi_radix_sorter.
 */
struct mpi_radix_sorter_config {
  size_t* data;            /// Array to sort.
  size_t  n_elts;          /// # elements to sort.
  size_t  base;            /// base for sort (10, 16, etc.).
  int     mpi_rank;        /// Rank in MPI_COMM_WORLD .
  int     mpi_world_size;  /// Total # processes.
};

/**
 * \brief Handle for MPI implementation of radix sort.
 */
struct mpi_radix_sorter {
  size_t* data;      /// The data a rank is working on.
  size_t* cum_data;  /// The total data to be sorted. Only valid at root.
  size_t  n_elts;    /// Total # elements. Valid at all ranks */
  size_t  base;      /// base for sort (10, 16, etc.).

  size_t  chunk_size;  /// Size of each worker chunk. Valid at all ranks.
  int     mpi_rank;
  int     mpi_world_size;
  size_t* tmp_arr;  /// tmp array used for doing the actual sort.

  /**
   * Prefix sums for each rank for each symbol in a digit.
   */
  size_t* prefix_sums;

  /**
   * Cumulative prefix sums used by the master to compute receive and
   * displacement counts.
   */
  size_t* cum_prefix_sums;
};

/*******************************************************************************
 * Public API
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Initialize a multi-process radix sorter.
 *
 * \param config Initialization parameters.
 *
 * \return The initialized sorter, or NULL if an error occurred.
 */
RCSW_API struct mpi_radix_sorter* mpi_radix_sorter_init(
  const struct mpi_radix_sorter_config* const config) RCSW_WUR;

/**
 * \brief Deallocate/destroy a sorter after use.
 *
 * \param sorter The sorter to destroy.
 */
RCSW_API void mpi_radix_sorter_destroy(struct mpi_radix_sorter* const sorter);

/**
 * \brief Perform radix sort in parallel using MPI.
 *
 * \param sorter The initialized sorter.
 *
 * \return \ref status_t.
 */
RCSW_API status_t mpi_radix_sorter_exec(struct mpi_radix_sorter* const sorter);

END_C_DECLS
