/**
 * \file
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * \ingroup algorithm
 *
 * \brief Collection of search algorithms. Binary search in iterative and
 * recursive variants.
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/al/types.h"
#include "rcsw/core/compilers.h"

/*******************************************************************************
 * Public API
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Search a sorted array using binary search (iterative).
 *
 * O(log n). Preferred over \ref bsearch_rec() for large arrays as it
 * does not grow the stack.
 *
 * \warning The array must be sorted in the order defined by \p cmpe.
 *          Passing an unsorted array produces undefined results.
 *
 * \param a      The array to search.
 * \param e      The element to search for.
 * \param cmpe   Comparison callback. Must return <0, 0, or >0 per the
 *               standard comparator convention.
 * \param el_size Size of each element in bytes.
 * \param low    Lowest index to consider (typically 0).
 * \param high   Highest index to consider (typically max index of array).
 *
 * \return Index of the matching element, or -1 if not found or on error.
 */
RCSW_API int bsearch_iter(const void* a,
                          const void* e,
                          int (*cmpe)(const void* e1, const void* e2),
                          size_t el_size,
                          int    low,
                          int    high);

/**
 * \brief Search a sorted array using binary search (recursive).
 *
 * O(log n). Stack depth is O(log n); prefer \ref bsearch_iter() for
 * large arrays or constrained stacks.
 *
 * \warning The array must be sorted in the order defined by \p cmpe.
 *          Passing an unsorted array produces undefined results.
 *
 * \param arr     The array to search.
 * \param e       The element to search for.
 * \param cmpe    Comparison callback. Must return <0, 0, or >0 per the
 *                standard comparator convention.
 * \param elt_size Size of each element in bytes.
 * \param low     Lowest index to consider (typically 0).
 * \param high    Highest index to consider (typically max index of array).
 *
 * \return Index of the matching element, or -1 if not found or on error.
 */
RCSW_API int bsearch_rec(const void* arr,
                         const void* e,
                         int (*cmpe)(const void* e1, const void* e2),
                         size_t elt_size,
                         int    low,
                         int    high);
END_C_DECLS
