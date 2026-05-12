/**
 * \file
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/algorithm/sort.h"

#include <stdlib.h>
#include <string.h>

#include "rcsw/algorithm/algorithm.h"
#include "rcsw/core/fpc.h"
#include "rcsw/ds/ds.h"
#include "rcsw/ds/llist.h"
#include "rcsw/er/client.h"

/******************************************************************************
 * Constants
 ******************************************************************************/
#define RCSW_SORT_MAX_ELT_SIZE 64
#define RCSW_SORT_MAX_STACK_DEPTH 4096

/*******************************************************************************
 * Private API
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Partition an array in quicksort
 *
 * This function partitions an array for quicksort(). It uses two pointers: one
 * moving in from the left and a second moving in from the right. They are moved
 * towards the center until the left pointer finds an element greater than the
 * pivot and the right one finds an element less than the pivot. These two
 * elements are then swapped. The pointers are then moved inward again until
 * they "cross over". The pivot is then swapped into the slot to which the right
 * pointer points and the partition is complete.
 *
 * Currently, this function always uses the lowest index as the pivot, which
 * can result in the worst case O(n^2) complexity if the array is already
 * sorted. I'll fix this eventually...
 *
 * \param a The array to partition
 * \param min_index Starting index
 * \param max_index Ending index
 * \param elt_size Size of elements in bytes
 * \param cmpe Function to compare 2 elements
 *
 * \return The partition index
 */
static int partition(void* const a,
                     int         min_index,
                     int         max_index,
                     size_t      elt_size,
                     int (*cmpe)(const void* const e1, const void* const e2)) {
  int left  = min_index; /* index starts at min_index and increases */
  int right = max_index; /* index starts and max_index and decreases */
  RCSW_CHECK(elt_size <= RCSW_SORT_MAX_ELT_SIZE);

  uint8_t* const arr = a;

  /* chose pivot element */
  uint8_t* const pivot = arr + (min_index * (int)elt_size);

  uint8_t tmp[RCSW_SORT_MAX_ELT_SIZE];

  while (left < right) {
    while (cmpe(arr + (left * (int)elt_size), pivot) <= 0 && left < max_index) {
      left++;
    }

    /* move right while item > pivot */
    while (cmpe(arr + (right * (int)elt_size), pivot) > 0 && right > 0) {
      right--;
    }
    /*
     * At this point arr[left] must be > pivot and arr[right] must be < pivot,
     * so swap them if the position pointers have not crossed.
     */
    if (left < right) {
      memmove(&tmp, arr + (left * (int)elt_size), elt_size);
      memmove(arr + (left * (int)elt_size),
              arr + (right * (int)elt_size),
              elt_size);
      memmove(arr + (right * (int)elt_size), &tmp, elt_size);
    }
  } /* while() */

  /*
   * arr[right] is <= pivot and in the upper half so swap it and the
   * item in the first position.
   */
  memmove(&tmp, pivot, elt_size);
  memmove(arr + (min_index * (int)elt_size),
          arr + (right * (int)elt_size),
          elt_size);
  memmove(arr + (right * (int)elt_size), &tmp, elt_size);

error:
  return right;
}

/*******************************************************************************
 * Public API
 ******************************************************************************/
status_t qsort_rec(void* const a,
                   int         min_index,
                   int         max_index,
                   size_t      elt_size,
                   int (*cmpe)(const void* const e1, const void* const e2)) {
  status_t ret = OK;
  if (max_index > min_index) {
    int pivot = partition(a, min_index, max_index, elt_size, cmpe);
    ret |= qsort_rec(a, min_index, pivot - 1, elt_size, cmpe);
    ret |= qsort_rec(a, pivot + 1, max_index, elt_size, cmpe);
  }
  return ret;
}

status_t qsort_iter(void* const a,
                    int         max_index,
                    size_t      elt_size,
                    int (*cmpe)(const void* const e1, const void* const e2)) {
  RCSW_FPC_NV(ERROR, elt_size <= RCSW_SORT_MAX_ELT_SIZE, max_index > 0);
  int min_index = 0;

  /*
   * Create an auxiliary stack. This is used to emulate the call stack for a
   * recursive implementation of quicksort.
   */
  int stack[RCSW_SORT_MAX_STACK_DEPTH * 2];

  /*
   * Initialize top of stack. In this case, the top points to the topmost USED
   * element on the stack.
   */
  int top = -1;

  /* push initial values of min_index and max_index to stack */
  stack[++top] = min_index;
  stack[++top] = max_index;

  /* while the stack is not empty you are not done sorting */
  while (top >= 0) {
    /* pop new min_index and max_index */
    max_index = stack[top--];
    min_index = stack[top--];

    /* get pivot position */
    int p = partition(a, min_index, max_index, elt_size, cmpe);

    /*
     * If there are elements on left side of pivot, then push left sub-array
     * onto stack to be sorted (this is the equivalent of a recursive call)
     */
    if (p - 1 > min_index) {
      stack[++top] = min_index; /* new min_index */
      stack[++top] = p - 1;     /* new max_index */
    }

    /*
     * If there are elements on right side of pivot, then push right sub-array
     * onto stack to be sorted (this is the equivalent of a recursive call)
     */
    if (p + 1 < max_index) {
      stack[++top] = p + 1;     /* new min_index */
      stack[++top] = max_index; /* new max_index */
    }
  } /* while (top >= 0) */
  return OK;
}

status_t insertion_sort(void*  arr,
                        size_t n_elts,
                        size_t elt_size,
                        int (*cmpe)(const void* const e1, const void* const e2)) {
  RCSW_FPC_NV(ERROR,
              NULL != arr,
              elt_size > 0,
              elt_size < RCSW_SORT_MAX_ELT_SIZE);
  if (1 == n_elts) {
    return OK;
  }
  /*
   * The element at j is the element you are currently comparing with/the
   * temporary element. Start at index j-1, move downward through the array,
   * until you find an element that is NOT > the element at j, and swap j
   * into that position.
   */
  uint8_t key[RCSW_SORT_MAX_ELT_SIZE];

  int i, j;
  for (i = 1; i < (int)n_elts; ++i) {
    memcpy(key, (uint8_t*)arr + (i * (int)elt_size), elt_size);
    j = i - 1;
    while (j >= 0 && cmpe((uint8_t*)arr + (j * (int)elt_size), key) > 0) {
      memmove((uint8_t*)arr + ((j + 1) * (int)elt_size),
              (uint8_t*)arr + (j * (int)elt_size),
              elt_size);
      --j;
    } /* while() */
    memcpy((uint8_t*)arr + ((j + 1) * (int)elt_size), key, elt_size);
  } /* for(j..) */
  return OK;
}

status_t radix_sort(size_t* const arr,
                    size_t* const tmp,
                    size_t        n_elts,
                    size_t        base) {
  status_t ret = OK;
  /* get largest # in array to get total # of digits */
  size_t m = alg_arr_largest_num(arr, n_elts);

  /* Do counting sort on each digit */
  for (size_t exp = 1; m / exp > 0; exp *= base) {
    ret |= radix_counting_sort(arr, tmp, n_elts, exp, base);
  } /* for(exp...) */
  return ret;
}

status_t radix_sort_prefix_sum(const size_t* const arr,
                               size_t              n_elts,
                               size_t              digit,
                               size_t              base,
                               size_t* const       prefix_sums) {
  RCSW_FPC_NV(ERROR, NULL != arr, n_elts > 0, base > 0, NULL != prefix_sums);
  memset(prefix_sums, 0, sizeof(size_t) * base);

  /*
   * Count how many occurrences of each possible value of the base in the
   * current digit
   */
  for (size_t i = 0; i < n_elts; i++) {
    prefix_sums[(arr[i] / digit) % base]++;
  } /* for(i..) */

  /* Update count to contain prefix sums in each element */
  for (size_t i = 1; i < base; i++) {
    prefix_sums[i] += prefix_sums[i - 1];
  } /* for(i..) */
  return OK;
}

status_t radix_counting_sort(size_t* const arr,
                             size_t* const tmp,
                             size_t        n_elts,
                             size_t        digit,
                             size_t        base) {
  RCSW_FPC_NV(ERROR, NULL != arr, NULL != tmp, n_elts > 0, digit > 0, base > 0);

  size_t prefix_sums[16];
  memset(prefix_sums, 0, sizeof(prefix_sums));
  memset(tmp, 0, sizeof(size_t) * n_elts);

  /* compute prefix sums for current digit */
  radix_sort_prefix_sum(arr, n_elts, digit, base, prefix_sums);

  /* Sort elements */
  for (size_t i = n_elts; i-- > 0;) {
    tmp[prefix_sums[(arr[i] / digit) % base] - 1] = arr[i];
    prefix_sums[(arr[i] / digit) % base]--;
  } /* for(i..) */

  /* Copy back to original array */
  for (size_t i = 0; i < n_elts; i++) {
    arr[i] = tmp[i];
  } /* for(i..) */

  return OK;
}

END_C_DECLS
