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
#include "rcsw/utils/byteops.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "rcsw/core/fpc.h"

/*******************************************************************************
 * Public API
 ******************************************************************************/
BEGIN_C_DECLS
status_t utils_string_gen(char* const buf, size_t len) {
  RCSW_FPC_NV(ERROR, buf != NULL, len > 0);

  /* ASCII characters 33 to 126 */
  size_t n_chars = len - 1;
  size_t i;
  for (i = 0; i < n_chars; ++i) {
    buf[i] = (char)((random() % (126 - 33 + 1)) + 33);
  }
  buf[n_chars] = '\0';

  return OK;
}

void utils_arr8_reverse(void* const arr, size_t size) {
  if (size < 2) {
    return;
  }
  for (size_t i = 0, j = size - 1; i < j; i++, j--) {
    uint8_t tmp        = ((uint8_t*)arr)[i];
    ((uint8_t*)arr)[i] = ((uint8_t*)arr)[j];
    ((uint8_t*)arr)[j] = tmp;
  } /* for() */
}

void utils_elt_swap(void* const elt1, void* const elt2, size_t elt_size) {
  RCSW_FPC_V(NULL != elt1, NULL != elt2);

  if (elt1 == elt2) {
    return;
  }
  /* Through a small bounce buffer: no VLA, no element size limit */
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
}

END_C_DECLS
