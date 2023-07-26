/**
 * \file search.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/algorithm/search.h"

#include <math.h>

#include "rcsw/common/fpc.h"
#include "rcsw/er/client.h"

/*******************************************************************************
 * Functions
 ******************************************************************************/
BEGIN_C_DECLS

int bsearch_iter(const void* const a,
                 const void* const e,
                 int (*cmpe)(const void* const e1, const void* const e2),
                 size_t elt_size,
                 int low,
                 int high) {
  RCSW_FPC_NV(-1, NULL != a, NULL != e, NULL != cmpe);

  const uint8_t* const arr = a;
  while (low <= high) {
    int index = (low + high) / 2;
    if (cmpe(arr + (index * elt_size), e) == 0) { /* found a match */
      return (int)index;
    } else if (cmpe(e, arr + (index * elt_size)) < 0) { /* left half */
      high = index - 1;
    } else { /* right half */
      low = index + 1;
    }
  } /* while() */
  errno = EAGAIN;
  return -1;
} /* bsearch_iter() */

int bsearch_rec(const void* const in,
                const void* const elt,
                int (*cmpe)(const void* const e1, const void* const e2),
                size_t elt_size,
                int low,
                int high) {
  RCSW_FPC_NV(-1, NULL != in, NULL != elt, NULL != cmpe);

  /*
   * We want indices, BUT if we get handed an array with 0 elements, then low
   * will be 0 and high will probably be n_elts - 1, and since n_elts=0, this
   * will be a huge number in size_t.
   */
  if (low > high) {
    return -1;
  }
  int mid = (high + low) / 2;
  const uint8_t* const arr = in;
  int rval = cmpe(elt, arr + (elt_size * mid));

  if (0 == rval) { /* found a match */
    return mid;
  } else if (rval < 0) { /* lower half */
    if (low == mid) {
      return -1; /* no match */
    } else {
      return bsearch_rec(arr, elt, cmpe, elt_size, low, mid - 1);
    }

  } else { /* upper half */
    if (high == mid) {
      return -1; /* no match */
    } else {
      return bsearch_rec(arr, elt, cmpe, elt_size, mid + 1, high);
    }
  }
} /* bsearch_rec() */

END_C_DECLS
