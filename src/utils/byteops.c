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
  RCSW_FPC_NV(ERROR, buf != NULL);

  /* ASCII characters 33 to 126 */
  size_t n_chars = len - 1;
  size_t i;
  for (i = 0; i < n_chars; ++i) {
    buf[i] = (char)(rand() % (126 - 33 + 1) + 33);
  }
  buf[n_chars] = '\0';

  return OK;
}

void utils_arr8_reverse(void* const arr, size_t size) {
  for (size_t i = 0, j = size - 1; i < j; i++, j--) {
    uint8_t tmp        = ((uint8_t*)arr)[i];
    ((uint8_t*)arr)[i] = ((uint8_t*)arr)[j];
    ((uint8_t*)arr)[j] = tmp;
  } /* for() */
}

void utils_elt_swap(void* arr, size_t elt_size, size_t i, size_t j) {
  if (i == j) {
    return;
  }
  uint8_t  tmp[elt_size];
  uint8_t* base = arr;
  memcpy(tmp, base + i * elt_size, elt_size);
  memcpy(base + i * elt_size, base + j * elt_size, elt_size);
  memcpy(base + j * elt_size, tmp, elt_size);
}

END_C_DECLS
