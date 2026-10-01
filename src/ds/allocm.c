/**
 * \file
 *
 * \copyright 2023 John Harwell
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/allocm.h"

/*******************************************************************************
 * Public API
 ******************************************************************************/
BEGIN_C_DECLS

/* Entry 0 is the free list head; block i is described by entry i + 1 */
#define ALLOCM_HEAD(map) ((map)[0].value)
#define ALLOCM_BLOCK(map, i) ((map)[(i) + 1].value)
#define ALLOCM_END (-1)

void allocm_init(struct allocm_entry* map, size_t n_blocks) {
  /* chain all blocks in index order: 0 -> 1 -> ... -> n-1 -> end */
  for (size_t i = 0; i < n_blocks; ++i) {
    ALLOCM_BLOCK(map, i) = (i + 1 < n_blocks) ? (int32_t)(i + 1) : ALLOCM_END;
  }
  ALLOCM_HEAD(map) = (n_blocks > 0) ? 0 : ALLOCM_END;
} /* allocm_init() */

int allocm_alloc(struct allocm_entry* map) {
  int32_t idx = ALLOCM_HEAD(map);
  if (ALLOCM_END == idx) {
    return -1;
  }
  ALLOCM_HEAD(map)       = ALLOCM_BLOCK(map, idx);
  ALLOCM_BLOCK(map, idx) = RCSW_ALLOCM_INUSE;
  return idx;
} /* allocm_alloc() */

void allocm_free(struct allocm_entry* map, size_t index) {
  if (RCSW_ALLOCM_INUSE != ALLOCM_BLOCK(map, index)) {
    return; /* already free: pushing it again would create a cycle */
  }
  ALLOCM_BLOCK(map, index) = ALLOCM_HEAD(map);
  ALLOCM_HEAD(map)         = (int32_t)index;
} /* allocm_free() */

END_C_DECLS
