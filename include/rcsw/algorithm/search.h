/**
 * \file search.h
 * \ingroup algorithm
 * \brief Collection of various search algorithms.
 *
 * Binary search (iterative and recursive).
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/rcsw.h"

/*******************************************************************************
 * API Functions
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
 *
 * \param low Lowest index in the array to consider when searching. This should
 *            usually be 0.
 *
 * \param high Highest index in the array to consider when searching. This
 *             should usually be max index of the array.
 *
 * \return The index, or -1 if not found or an ERROR occurred
 */
RCSW_API int bsearch_iter(const void * a,
                 const void * e,
                 int (*cmpe)(const void * e1, const void * e2),
                 size_t el_size,
                 int low,
                 int high);

/**
 * \brief Search a sorted array of data using binary search, recursive version
 *
 * \param arr The array to search.
 *
 * \param e The element to search for.
 *
 * \param cmpe Callback to compare two elements.
 *
 * \param elt_size Size of elements in bytes.
 *
 * \param low Lowest index in the array to consider when searching. This should
 *            usually be 0.
 *
 * \param high Highest index in the array to consider when searching. This
 *             should usually be max index of the array.
 *
 * \return The index, or -1 if not found/an error occurred.
 *
 */
RCSW_API int bsearch_rec(const void * arr,
                const void * e,
                int (*cmpe)(const void * e1, const void * e2),
                size_t elt_size,
                int low,
                int high);
END_C_DECLS
