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
#include "rcsw/common/dbg.h"
#include "rcsw/common/fpc.h"

/*******************************************************************************
 * Functions
 ******************************************************************************/
BEGIN_C_DECLS

int bsearch_iter(const void* const a,
                 const void* const e,
                 int (*cmpe)(const void* const e1, const void* const e2),
                 size_t el_size,
                 size_t high,
                 size_t low) {
  RCSW_FPC_NV(-1, NULL != a, NULL != e, NULL != cmpe);

  const uint8_t* const arr = a;
  while (low <= high) {
    size_t index = (low + high) / 2;
    if (cmpe(arr + (index * el_size), e) == 0) { /* found a match */
      return (int)index;
    } else if (cmpe(e, arr + (index * el_size)) < 0) { /* left half */
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
                size_t el_size,
                size_t low,
                size_t high) {
  RCSW_FPC_NV(-1, NULL != in, NULL != elt, NULL != cmpe);
  /* printf("low: %zu, high: %zu\n", low,high); */
  if (low > high) {
    return -1;
  }
  size_t mid = (high + low) / 2;
  const uint8_t* const arr = in;
  int rval = cmpe(elt, arr + (el_size * mid));

  if (0 == rval) { /* found a match */
    return (int)mid;
  } else if (rval < 0) { /* lower half */
    if (low == mid) {
      return -1; /* no match */
    } else {
      return bsearch_rec(arr, elt, cmpe, el_size, low, mid - 1);
    }

  } else { /* upper half */
    if (high == mid) {
      return -1; /* no match */
    } else {
      return bsearch_rec(arr, elt, cmpe, el_size, mid + 1, high);
    }
  }
} /* bsearch_rec() */

END_C_DECLS
