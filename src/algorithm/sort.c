/**
 * \file
 *
 * \copyright 2017 John Harwell
 *
 * SPDX-License-Identifier: MIT
 */
/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/algorithm/sort.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "rcsw/core/fpc.h"
#include "rcsw/ds/ds.h"
#include "rcsw/ds/llist.h"
#include "rcsw/er/client.h"
#include "rcsw/er/macros.h"

/*******************************************************************************
 * Constants
 ******************************************************************************/
/*
 * Explicit stack for qsort_iter(). Because the smaller partition is always
 * processed first, the stack never holds more than ~log2(n) partitions, and
 * n is bounded by INT_MAX, so this can never overflow.
 */
#define RCSW_SORT_STACK_DEPTH ((sizeof(int) * CHAR_BIT) + 1)

/*******************************************************************************
 * Private API
 ******************************************************************************/
BEGIN_C_DECLS
/**
 * \brief Find the largest value in an array of size_t.
 */
static size_t arr_largest(const size_t* const array, size_t n_elts) {
  size_t largest = 0;
  for (size_t i = 0; i < n_elts; i++) {
    if (array[i] > largest) {
      largest = array[i];
    }
  } /* for(i..) */
  return largest;
} /* arr_largest() */

/**
 * \brief Swap two non-overlapping elements of any size.
 */
static void sort_swap(uint8_t* a, uint8_t* b, size_t elt_size) {
  uint8_t chunk[32];
  while (elt_size > 0) {
    size_t n = RCSW_MIN(elt_size, sizeof(chunk));
    memcpy(chunk, a, n);
    memcpy(a, b, n);
    memcpy(b, chunk, n);
    a += n;
    b += n;
    elt_size -= n;
  } /* while() */
} /* sort_swap() */

/**
 * \brief Move the median of the first, middle, and last elements of [lo, hi]
 * to index lo, for use as the quicksort pivot.
 *
 * Median-of-three makes already-sorted and reverse-sorted input O(n log n)
 * instead of O(n^2).
 */
static void pivot_select(uint8_t* const arr,
                         int            lo,
                         int            hi,
                         size_t         elt_size,
                         int (*cmpe)(const void* const e1,
                                     const void* const e2)) {
  int      mid = lo + ((hi - lo) / 2);
  uint8_t* a   = arr + ((size_t)lo * elt_size);
  uint8_t* b   = arr + ((size_t)mid * elt_size);
  uint8_t* c   = arr + ((size_t)hi * elt_size);

  /* order a <= b <= c, then the median is in b */
  if (cmpe(b, a) < 0) {
    sort_swap(a, b, elt_size);
  }
  if (cmpe(c, b) < 0) {
    sort_swap(b, c, elt_size);
    if (cmpe(b, a) < 0) {
      sort_swap(a, b, elt_size);
    }
  }
  if (mid != lo) {
    sort_swap(a, b, elt_size);
  }
} /* pivot_select() */

/**
 * \brief Partition [min_index, max_index] around a median-of-three pivot.
 *
 * Hoare-style partitioning: a pointer moving in from the left stops at the
 * first element >= the pivot, and one moving in from the right stops at the
 * first element <= the pivot; the two are swapped and the scans continue until
 * the pointers cross. Stopping on equal elements on both sides splits runs of
 * equal keys evenly, so arrays with many duplicates do not degrade to O(n^2).
 *
 * \return The final index of the pivot. Everything before it is <= the pivot,
 * everything after it is >= the pivot.
 */
static int partition(void* const a,
                     int         min_index,
                     int         max_index,
                     size_t      elt_size,
                     int (*cmpe)(const void* const e1, const void* const e2)) {
  uint8_t* const arr = a;
  pivot_select(arr, min_index, max_index, elt_size, cmpe);

  uint8_t* const pivot = arr + ((size_t)min_index * elt_size);
  int            left  = min_index;
  int            right = max_index + 1;

  for (;;) {
    do {
      ++left;
    } while (left <= max_index &&
             cmpe(arr + ((size_t)left * elt_size), pivot) < 0);
    do {
      --right; /* the pivot itself stops this scan */
    } while (cmpe(arr + ((size_t)right * elt_size), pivot) > 0);

    if (left >= right) {
      break;
    }
    sort_swap(arr + ((size_t)left * elt_size),
              arr + ((size_t)right * elt_size),
              elt_size);
  } /* for(;;) */

  if (right != min_index) {
    sort_swap(pivot, arr + ((size_t)right * elt_size), elt_size);
  }
  return right;
} /* partition() */

/*******************************************************************************
 * Public API
 ******************************************************************************/
status_t qsort_rec(void* const a,
                   int         min_index,
                   int         max_index,
                   size_t      elt_size,
                   int (*cmpe)(const void* const e1, const void* const e2)) {
  RCSW_FPC_NV(ERROR, NULL != a, elt_size > 0, min_index >= 0);
  ER_ASSERT(NULL != cmpe, "qsort_rec() requires cmpe()");

  /*
   * Recurse into the smaller side and loop on the larger one, so recursion
   * depth is at most log2(n).
   */
  while (max_index > min_index) {
    int pivot = partition(a, min_index, max_index, elt_size, cmpe);
    if (pivot - min_index < max_index - pivot) {
      RCSW_CHECK(OK == qsort_rec(a, min_index, pivot - 1, elt_size, cmpe));
      min_index = pivot + 1;
    } else {
      RCSW_CHECK(OK == qsort_rec(a, pivot + 1, max_index, elt_size, cmpe));
      max_index = pivot - 1;
    }
  } /* while() */
  return OK;

error:
  return ERROR;
} /* qsort_rec() */

status_t qsort_iter(void* const a,
                    int         max_index,
                    size_t      elt_size,
                    int (*cmpe)(const void* const e1, const void* const e2)) {
  RCSW_FPC_NV(ERROR, NULL != a, elt_size > 0, max_index >= 0);
  ER_ASSERT(NULL != cmpe, "qsort_iter() requires cmpe()");

  /* [min, max] index pairs still to be partitioned */
  int stack[RCSW_SORT_STACK_DEPTH][2];
  int top = 0;

  stack[top][0] = 0;
  stack[top][1] = max_index;
  ++top;

  while (top > 0) {
    --top;
    int min_index = stack[top][0];
    max_index     = stack[top][1];

    while (min_index < max_index) {
      int p = partition(a, min_index, max_index, elt_size, cmpe);

      /*
       * Defer the larger side and keep working on the smaller side, so at
       * most log2(n) partitions are ever pending.
       */
      if (p - min_index < max_index - p) {
        stack[top][0] = p + 1;
        stack[top][1] = max_index;
        max_index     = p - 1;
      } else {
        stack[top][0] = min_index;
        stack[top][1] = p - 1;
        min_index     = p + 1;
      }
      ++top;
    } /* while(min_index < max_index) */
  } /* while(top > 0) */
  return OK;
} /* qsort_iter() */

status_t insertion_sort(void*  arr,
                        size_t n_elts,
                        size_t elt_size,
                        int (*cmpe)(const void* const e1, const void* const e2)) {
  RCSW_FPC_NV(ERROR, NULL != arr, elt_size > 0);
  ER_ASSERT(NULL != cmpe, "insertion_sort() requires cmpe()");

  uint8_t* const a = arr;
  /*
   * Sink element i into place by swapping it down past every larger element
   * before it. Swapping adjacent elements avoids a temporary of elt_size.
   */
  for (size_t i = 1; i < n_elts; ++i) {
    for (size_t j = i; j > 0; --j) {
      uint8_t* prev = a + ((j - 1) * elt_size);
      uint8_t* curr = a + (j * elt_size);
      if (cmpe(prev, curr) <= 0) {
        break;
      }
      sort_swap(prev, curr, elt_size);
    } /* for(j..) */
  } /* for(i..) */
  return OK;
} /* insertion_sort() */

status_t radix_sort(size_t* const arr,
                    size_t* const tmp,
                    size_t        n_elts,
                    size_t        base) {
  RCSW_FPC_NV(ERROR, NULL != arr, NULL != tmp, base >= 2, base <= 16);
  if (0 == n_elts) {
    return OK; /* nothing to sort */
  }

  /* get largest # in array to get total # of digits */
  size_t m = arr_largest(arr, n_elts);

  /* Do counting sort on each digit */
  for (size_t exp = 1; m / exp > 0; exp *= base) {
    RCSW_CHECK(OK == radix_counting_sort(arr, tmp, n_elts, exp, base));
    if (exp > SIZE_MAX / base) {
      break; /* no higher digit exists */
    }
  } /* for(exp...) */
  return OK;

error:
  return ERROR;
} /* radix_sort() */

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
  RCSW_FPC_NV(ERROR,
              NULL != arr,
              NULL != tmp,
              n_elts > 0,
              digit > 0,
              base >= 2,
              base <= 16);

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
