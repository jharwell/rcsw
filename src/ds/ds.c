/**
 * \file ds.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/ds.h"

#include <math.h>

#include "rcsw/er/client.h"
#include "rcsw/common/fpc.h"

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

#ifndef __SIZEOF_FLOAT__
#warning "__SIZEOF_FLOAT__ not defined--strange behavior may occur..."
#endif

status_t ds_elt_copy(void* const elt1, const void* const elt2, size_t elt_size) {
  RCSW_FPC_NV(ERROR, NULL != elt1, NULL != elt2, elt_size > 0);

  switch (elt_size) {
    case sizeof(uint8_t):
      *((uint8_t*)(elt1)) = *((const uint8_t*)(elt2));
      break;
    case sizeof(uint16_t):
      *((uint16_t*)(elt1)) = *((const uint16_t*)(elt2));
      break;
    case sizeof(uint32_t):
      *((uint32_t*)(elt1)) = *((const uint32_t*)(elt2));
      break;
#if __SIZEOF_FLOAT__ != 4
    case sizeof(float):
      *((float*)(elt1)) = *((const float*)(elt2));
      break;
#endif
    case sizeof(double):
      *((double*)(elt1)) = *((const double*)(elt2));
      break;
    default:
      memcpy(elt1, elt2, elt_size);
      break;
  }

  return OK;
} /* ds_elt_copy() */

status_t ds_elt_swap(void* const elt1, void* const elt2, size_t elt_size) {
  RCSW_FPC_NV(
      ERROR, NULL != elt1, NULL != elt2, elt_size > 0, elt_size <= sizeof(double));
  double tmp;
  switch (elt_size) {
    case sizeof(uint8_t):
      *((uint8_t*)(elt1)) ^= *((const uint8_t*)(elt2));
      *((uint8_t*)(elt2)) ^= *((const uint8_t*)(elt1));
      *((uint8_t*)(elt1)) ^= *((const uint8_t*)(elt2));
      break;
    case sizeof(uint16_t):
      *((uint16_t*)(elt1)) ^= *((const uint16_t*)(elt2));
      *((uint16_t*)(elt2)) ^= *((const uint16_t*)(elt1));
      *((uint16_t*)(elt1)) ^= *((const uint16_t*)(elt2));
      break;
    case sizeof(uint32_t):
      *((uint32_t*)(elt1)) ^= *((const uint32_t*)(elt2));
      *((uint32_t*)(elt2)) ^= *((const uint32_t*)(elt1));
      *((uint32_t*)(elt1)) ^= *((const uint32_t*)(elt2));
      break;
#if __SIZEOF_FLOAT__ != 4
    case sizeof(float):
      tmp = *((float*)(elt1));
      *((float*)(elt1)) = *((const float*)(elt2));
      *((float*)(elt2)) = tmp;
      break;
#endif
    case sizeof(double):
      tmp = *((const double*)(elt1));
      *((double*)(elt1)) = *((const double*)(elt2));
      *((double*)(elt2)) = tmp;
      break;
    default:
      break;
  } /* switch() */

  return OK;
} /* ds_elt_swap() */

status_t ds_elt_clear(void* const elt, size_t elt_size) {
  RCSW_FPC_NV(ERROR, NULL != elt, elt_size > 0);

  switch (elt_size) {
    case sizeof(uint8_t):
      *((uint8_t*)(elt)) = 0;
      break;
    case sizeof(uint16_t):
      *((uint16_t*)(elt)) = 0;
      break;
    case sizeof(uint32_t):
      *((uint32_t*)(elt)) = 0;
      break;
/*
 * sizeof(float) is the same as sizeof(uint32_t) on most platforms, but
 * not all.
 */
#if __SIZEOF_FLOAT__ != 4
    case sizeof(float):
      *((float*)(elt)) = 0;
      break;
#endif
    case sizeof(double):
      *((double*)(elt)) = 0;
      break;
    default:
      memset(elt, 0, elt_size);
      break;
  } /* switch() */

  return OK;
} /* ds_elt_clear() */

bool_t ds_elt_zchk(void* const elt, size_t elt_size) {
  RCSW_FPC_NV(false, NULL != elt, elt_size > 0);
  int sum = 0;

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
      return fabs(*((double *)(elt)) <= RCSW_FLOAT_EPSILON;
        break;
#endif
    case sizeof(double):
        return fabs(*((double *)(elt))) <= RCSW_DOUBLE_EPSILON;
        break;
    default:
        for (size_t i = 0; i < elt_size; ++i) {
        sum |= ((int*)elt)[i];
        } /* for(i..) */
        return sum == 0;
  } /* switch() */
} /* ds_elt_zchk() */

END_C_DECLS
