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
  bool_t res = 0;

  switch (elt_size) {
    case sizeof(uint8_t):
      return *((uint8_t*)(elt)) == 0;
    case sizeof(uint16_t):
      return *((uint16_t*)(elt)) == 0;
    case sizeof(uint32_t):
      return *((uint32_t*)(elt)) == 0;

      /*
       * sizeof(float) is the same as sizeof(uint32_t) on most platforms, but
       * not all.
       */
#if __SIZEOF_FLOAT__ != 4
    case sizeof(float):
      return fabs(*((float*)(elt))) <= RCSW_FLOAT_TOL;
      break;
#endif
    case sizeof(double):
      return fabs(*((double*)(elt))) <= RCSW_DOUBLE_TOL;
      break;
    default:
      for (size_t i = 0; i < elt_size; ++i) {
        res |= ((uint8_t*)elt)[i];
      } /* for(i..) */
      return res == 0;
  } /* switch() */
}

END_C_DECLS
