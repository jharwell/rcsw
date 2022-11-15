/**
 * \file search.c
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

int bsearch_rec(const void* const a,
                const void* const e,
                int (*cmpe)(const void* const e1, const void* const e2),
                size_t el_size,
                size_t low,
                size_t high) {
  RCSW_FPC_NV(-1, NULL != a, NULL != e, NULL != cmpe);

  if (low > high) {
    return -1;
  }
  size_t mid = (high + low) / 2;
  const uint8_t* const arr = a;
  int rval = cmpe(e, arr + (el_size * mid));

  if (0 == rval) { /* found a match */
    return (int)mid;
  } else if (rval < 0) { /* lower half */
    return bsearch_rec(arr, e, cmpe, el_size, low, mid - 1);
  } else { /* upper half */
    return bsearch_rec(arr, e, cmpe, el_size, mid + 1, high);
  }
} /* bsearch_rec() */

END_C_DECLS
