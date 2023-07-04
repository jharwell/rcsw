/**
 * \file allocm.c
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/allocm.h"

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

void allocm_init(struct allocm_entry* map, size_t max_elts) {
  for (size_t i = 0; i < max_elts; ++i) {
    allocm_mark_free(map + i);
  }
} /* db_alloc_init() */

int allocm_probe(struct allocm_entry* map, size_t max_elts, size_t index) {
  bool_t wrap = false;
  size_t i = index;
  int ret = -1;

  while (!wrap || (i != index)) {
    if (map[i].value == -1) {
      ret = i;
      break;
    }
    /* wrapped around to index 0 */
    if (i + 1 == max_elts) {
      wrap = true;
      i = 0;
    } else {
      i++;
    }
  } /* while() */

  return ret;
} /* ds_meta_probe() */

END_C_DECLS
