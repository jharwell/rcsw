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
#include "rcsw/ds/ds.h"

#include <stdint.h>
#include <string.h>

#include "rcsw/core/core.h"
#include "rcsw/core/fpc.h"
#include "rcsw/er/client.h"

/*******************************************************************************
 * Public API
 ******************************************************************************/
BEGIN_C_DECLS

/*
 * These helpers operate on raw bytes via memcpy()/memset(). Element storage is
 * only guaranteed to be aligned to RCSW_CONFIG_PTR_ALIGN, so loading elements
 * through typed pointers (uint32_t*, double*, ...) is undefined behavior and
 * can fault on strict-alignment targets. Compilers inline small fixed-size
 * memcpy() calls, so nothing is lost.
 */
status_t ds_elt_copy(void* const elt1, const void* const elt2, size_t elt_size) {
  RCSW_FPC_NV(ERROR, NULL != elt1, NULL != elt2, elt_size > 0);

  if (elt1 != elt2) {
    memcpy(elt1, elt2, elt_size);
  }
  return OK;
} /* ds_elt_copy() */

status_t ds_elt_swap(void* const elt1, void* const elt2, size_t elt_size) {
  RCSW_FPC_NV(ERROR, NULL != elt1, NULL != elt2, elt_size > 0);

  if (elt1 == elt2) {
    return OK;
  }
  /* Swap through a small bounce buffer, so there is no element size limit */
  uint8_t* a = elt1;
  uint8_t* b = elt2;
  uint8_t  chunk[32];
  while (elt_size > 0) {
    size_t n = RCSW_MIN(elt_size, sizeof(chunk));
    memcpy(chunk, a, n);
    memcpy(a, b, n);
    memcpy(b, chunk, n);
    a += n;
    b += n;
    elt_size -= n;
  } /* while() */
  return OK;
} /* ds_elt_swap() */

status_t ds_elt_clear(void* const elt, size_t elt_size) {
  RCSW_FPC_NV(ERROR, NULL != elt, elt_size > 0);

  memset(elt, 0, elt_size);
  return OK;
} /* ds_elt_clear() */

END_C_DECLS
