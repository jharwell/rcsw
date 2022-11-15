/**
 * \file search.h
 * \ingroup algorithm
 * \brief Collection of various search algorithms.
 *
 * Binary search (iterative and recursive).
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

#ifndef INCLUDE_RCSW_ALGORITHM_SEARCH_H_
#define INCLUDE_RCSW_ALGORITHM_SEARCH_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/common/common.h"

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
/**
 * \brief The runtime implementation method to use for the binary search
 * algorithm.
 */
enum search_types {
  BSEARCH_ITER,  /// Use a recursive runtime implementation
  BSEARCH_REC    /// Use a iterative runtime implementation
};

/*******************************************************************************
 * Function prototypes
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Search a sorted array of data using binary search, iterative version
 *
 * The array is assumed to already be sorted.
 *
 * \param a The array to search
 * \param e The element to search for
 * \param cmpe Callback to compare two elements
 * \param el_size Size of elements in bytes
 * \param high Highest index in the array to consider when searching. This should usually be max
 * index of the array.
 * \param low Lowest index in the array to consider when searching. This should usually be 0.
 *
 * \return The index, or -1 if not found or an ERROR occurred
 */
int bsearch_iter(const void * a, const void * e,
                 int (*cmpe)(const void * e1, const void * e2),
                 size_t el_size, size_t high, size_t low);

/**
 * \brief Search a sorted array of data using binary search, recursive version
 *
 * \param a The array to search
 * \param e The element to search for
 * \param cmpe Callback to compare two elements
 * \param el_size Size of elements in bytes
 * \param high Highest index in the array to consider when searching. This should usually be max
 * index of the array.
 * \param low Lowest index in the array to consider when searching. This should usually be 0.
 *
 * \return The index, or -1 if not found or an ERROR occurred
 *
 */
int bsearch_rec(const void * a, const void * e,
                int (*cmpe)(const void * e1, const void * e2),
                size_t el_size, size_t low, size_t high);
END_C_DECLS

#endif /* INCLUDE_RCSW_ALGORITHM_SEARCH_H_  */
