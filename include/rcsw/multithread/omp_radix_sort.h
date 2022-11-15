/**
 * \file omp_radix_sort.h
 * \ingroup multithread
 * \brief Multiprocess radix sorter using OpenMP
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


#ifndef INCLUDE_RCSW_MULTITHREAD_OMP_RADIX_SORT_H_
#define INCLUDE_RCSW_MULTITHREAD_OMP_RADIX_SORT_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/common/common.h"
#include "rcsw/ds/fifo.h"

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/
/**
 * \brief Parameters for \ref omp_radix_sorter.
 */
struct omp_radix_sorter_params {
    size_t* data;      /// Data to sort.
    size_t n_elts;     /// # elements to sort.
    size_t base;       /// base for sort (10, 16, etc.).
    size_t n_threads;  /// # OpenMP threads to use for sorting.
};

/**
 * \brief Handle for OMP implementation of radix sort.
 */
struct omp_radix_sorter {
    size_t* data;       /// The data to sort.
    size_t n_elts;      /// Total # elements.
    size_t base;        /// base for sort (10, 16, etc.).
    size_t n_threads;   /// # OpenMP threads to use for sorting.
    size_t chunk_size;  /// Per-thread chunk size.
    struct fifo* bins;  /// The bins to place numbers into when sorting.

    /**
     * Cumulative prefix sums used to compute receive and displacement counts.
     */
    size_t *cum_prefix_sums;
};

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Initialize a multi-thread radix sorter.
 *
 * \param params Initialization parameters.
 *
 * \return The initialized sorter, or NULL if an error occurred.
 */
struct omp_radix_sorter* omp_radix_sorter_init(
    const struct omp_radix_sorter_params* const params) RCSW_CHECK_RET;

/**
 * \brief Deallocate/destroy a sorter after use.
 *
 * \param sorter The sorter to destroy.
 */
void omp_radix_sorter_destroy(struct omp_radix_sorter* const sorter);

/**
 * \brief Perform radix sort in parallel using OpenMP.
 *
 * \param sorter The initialized sorter.
 *
 * \return \ref status_t.
 */
status_t omp_radix_sorter_exec(struct omp_radix_sorter* const sorter);

END_C_DECLS

#endif /* INCLUDE_RCSW_MULTITHREAD_OMP_RADIX_SORT_H_ */
