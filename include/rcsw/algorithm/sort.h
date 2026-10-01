/**
 * \file
 *
 * \copyright 2017 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * \ingroup algorithm
 *
 * \brief Collection of sorting algorithms.
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stddef.h>

#include "rcsw/core/compilers.h"
#include "rcsw/core/core.h"

/*******************************************************************************
 * Public API
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Sort an array using recursive quicksort.
 *
 * Average case O(n log n). The pivot is the median of the first, middle, and
 * last elements, and partitioning splits runs of equal keys evenly, so sorted,
 * reverse-sorted, and all-equal input are all O(n log n); O(n²) is possible
 * only for adversarially constructed input. Not stable. Recursion is only into
 * the smaller partition, so recursion depth is at most log2(n).
 *
 * \param a         The input array.
 * \param min_index Starting index (inclusive).
 * \param max_index Ending index (inclusive).
 * \param elt_size   Size of each element in bytes.
 * \param cmpe      Comparison function for elements.
 */
RCSW_API status_t qsort_rec(void*  a,
                            int    min_index,
                            int    max_index,
                            size_t elt_size,
                            int (*cmpe)(const void* const e1,
                                        const void* const e2));

/**
 * \brief Sort an array using iterative quicksort.
 *
 * Same pivot selection, partitioning, and complexity as \ref qsort_rec(), but
 * uses a fixed-size explicit stack instead of recursion: the smaller partition
 * is always processed first, so at most log2(n) partitions are pending.
 *
 * \param a         Array to sort.
 * \param max_index Maximum index to sort up to (i.e., sorts [0, max_index]).
 * \param elt_size   Size of each element in bytes.
 * \param cmpe      Comparison function for elements.
 */
RCSW_API status_t qsort_iter(void*  a,
                             int    max_index,
                             size_t elt_size,
                             int (*cmpe)(const void* const e1,
                                         const void* const e2));

/**
 * \brief Sort an array of non-negative integers using radix sort
 *
 * O(nk) time and space, where k is the number of digits in the maximum
 * element value for the given base. Non-comparative; integer keys only.
 *
 * \param arr    The array to sort.
 * \param tmp    Temporary array used during sorting. Must be at least as
 *               large as \p arr.
 * \param n_elts Number of elements in \p arr and \p tmp.
 * \param base   Numeric base for digit decomposition (e.g., 10, 8, 16).
 */
RCSW_API status_t radix_sort(size_t* arr,
                             size_t* tmp,
                             size_t  n_elts,
                             size_t  base);

/**
 * \brief Sort an array using insertion sort.
 *
 * O(n²) average and worst case. Only appropriate for very small arrays or
 * arrays that are known to be nearly sorted. For general use prefer \ref
 * qsort_rec() or \ref qsort_iter().
 *
 * \param arr      The array to sort.
 * \param n_elts   Number of elements in the array.
 * \param elt_size Size of each element in bytes.
 * \param cmpe     Comparison function for elements.
 */
RCSW_API status_t insertion_sort(void*  arr,
                                 size_t n_elts,
                                 size_t elt_size,
                                 int (*cmpe)(const void* const e1,
                                             const void* const e2));

/******************************************************************************
 * Private API
 ******************************************************************************/
/**
 * \brief Sort an array of non-negative integers via counting sort.
 *
 * Internal helper used by \ref radix_sort(). Processes one digit of each
 * element at a time.
 *
 * \param arr    The array to sort.
 * \param tmp    Temporary array; must be at least as large as \p arr.
 * \param n_elts Number of elements in \p arr and \p tmp.
 * \param digit  Current digit position being processed.
 * \param base   Numeric base; must be <= 16.
 *
 * \return \ref status_t
 */
RCSW_LOCAL status_t radix_counting_sort(
  size_t* arr, size_t* tmp, size_t n_elts, size_t digit, size_t base);

/**
 * \brief Compute prefix sums for one digit pass of radix sort.
 *
 * Internal helper used by \ref radix_sort(). Counts how many values are
 * less than arr[i] for the given digit position.
 *
 * \param arr         The array being sorted.
 * \param n_elts      Number of elements in \p arr.
 * \param digit       Current digit position being processed.
 * \param base        Numeric base; must be <= 16.
 * \param prefix_sums Output array (same size as \p arr) to receive prefix
 *                    sums.
 *
 * \return \ref status_t
 */
RCSW_LOCAL status_t radix_sort_prefix_sum(const size_t* arr,
                                          size_t        n_elts,
                                          size_t        digit,
                                          size_t        base,
                                          size_t*       prefix_sums);

END_C_DECLS
