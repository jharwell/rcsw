/**
 * \file sort.h
 * \ingroup algorithm
 * \brief Collection of sorting algorithms.
 *
 * Quicksort (iterative and recursive), mergesort (linked lists), radix sort
 * (arrays), counting sort (arrays).
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
 * Function prototypes
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Sort an array using quicksort
 *
 * This routine sorts an array by using a recursive implementation of
 * quicksort, rather than an iterative one, so some efficiency is lost.
 *
 * \param a The input array
 * \param min_index Starting index
 * \param max_index Ending index
 * \param el_size Size of elements in bytes
 * \param cmpe Comparision function for elements
 */
void qsort_rec(void *a, int min_index, int max_index,
               size_t el_size,
               int (*cmpe)(const void *const e1, const void *const e2));

/**
 * \brief Sort an array using quicksort
 *
 * This routine sorts an array by using an iterative, rather than a recursive,
 * implementation.
 *
 * \param a Array to sort
 * \param max_index Starting index
 * \param el_size  Size of elements in bytes
 * \param cmpe Comparision function for elements
 */
void qsort_iter(void *a, int max_index, size_t el_size,
                int (*cmpe)(const void *const e1, const void *const e2));

/**
 * \brief Sort a linked list using iterative mergesort
 *
 * This function sorts a linked list using at iterative implementation of
 * mergesort It has minimal stack/memory requirements, beyond a few local
 * variables.  It can be used to sort any singly or doubly linked list. It is
 * assumed that the list has at least 2 items in it.
 *
 * \param list The list to sort
 * \param cmpe A comparison function for the data managed by each node
 * \param isdouble true if the list to be sorted is doubly linked
 *
 * \return A pointer to the sorted list
 *
 */
struct llist_node *mergesort_iter(struct llist_node *list,
                                  int (*cmpe)(const void *const e1,
                                              const void *const e2),
                                  bool_t isdouble);

/**
  * \brief Sort a linked list using recursive mergesort
  *
  * This function sorts a linked list using a recursive implementation of
  * mergesort. It has a complexity of O(NLogN). It can be used to sort any
  * singly or doubly linked list.
  *
  * \param list The list to sort
  * \param cmpe A comparison function for the data managed by each node
  * \param isdouble true if the list to be sorted is doubly linked
  *
  * \return A pointer to the sorted list
  */
struct llist_node *mergesort_rec(struct llist_node *list,
                                 int (*cmpe)(const void *const e1,
                                             const void *const e2),
                                 bool_t isdouble);

/**
 * \brief Sort an array using insertion sort
 *
 * \param arr The array to sort
 * \param n_elts # elements in the array (must be >=3)
 * \param elt_size Size of elements in the array in bytes
 * \param cmpe Comparison function for elements
 */
void insertion_sort(void *arr, size_t n_elts, size_t elt_size,
                    int (*cmpe)(const void *const e1, const void *const e2));

/**
 * \brief Sort an array of non-negative integers using radix sort
 *
 * \param arr The array to sort
 *
 * \param tmp Temporary array to hold elements as they are sorted. Must be at
 *            least as large as the array to sort
 *
 * \param n_elts # elements in array
 *
 * \param base Base of numbers (10, 8, 16, etc.)
 */
void radix_sort(size_t *arr, size_t *tmp, size_t n_elts, size_t base);

/**
 * \brief Sort an array of non-negative ints via counting sort, as part of radix
 * sort
 *
 * \param arr The array to sort
 *
 * \param tmp Temporary array to hold elements as they are sorted. Must be at
 *            least as large as \p arr.
 *
 * \param n_elts # elements in \p arr and \p tmp.
 *
 * \param digit Current digit being processed.
 *
 * \param base The base of the numbers to be sorted; must be <= 16.
 *
 * \return \ref status_t
 */
status_t radix_counting_sort(size_t *arr,
                             size_t* tmp,
                             size_t n_elts,
                             size_t digit,
                             size_t base);

/**
 * \brief Count how many values are less than \p arr[i] for \p digit.
 *
 * \param arr The array to sort.
 *
 * \param n_elts # elements in \p arr.
 *
 * \param digit Current digit being processed.
 *
 * \param base The base of the numbers to be sorted; must be <= 16.
 *
 * \param prefix_sums Empty array of same cardinality as \p arr to store prefix
 *                    sums in.
 */
status_t radix_sort_prefix_sum(const size_t* arr,
                               size_t n_elts,
                               size_t digit,
                               size_t base,
                               size_t* prefix_sums);
END_C_DECLS
