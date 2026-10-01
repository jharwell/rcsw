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
#include "rcsw/utils/numeric.h"

#include <math.h>
#include <stddef.h>

#include "rcsw/core/fpc.h"
#include "rcsw/utils/byteops.h"

/*******************************************************************************
 * Public API
 ******************************************************************************/
BEGIN_C_DECLS

void utils_permute(void*  arr,
                   size_t n_elts,
                   size_t elt_size,
                   size_t start,
                   void (*fp)(void* arr)) {
  if (start == n_elts) {
    fp(arr);
    return;
  }
  for (size_t j = start; j < n_elts; ++j) {
    utils_elt_swap(arr, elt_size, start, j);
    utils_permute(arr, n_elts, elt_size, start + 1, fp);
    utils_elt_swap(arr, elt_size, start, j); /* restore */
  }
}

bool_t utils_zchk(void* const elt, size_t elt_size) {
  RCSW_FPC_NV(false, NULL != elt, elt_size > 0);

  /*
   * A byte-wise check: elements are opaque (an 8-byte element is not
   * necessarily a double), and need not be aligned for a typed load.
   */
  const uint8_t* bytes = elt;
  uint8_t        acc   = 0;
  for (size_t i = 0; i < elt_size; ++i) {
    acc |= bytes[i];
  } /* for(i..) */
  return 0 == acc;
}

END_C_DECLS
