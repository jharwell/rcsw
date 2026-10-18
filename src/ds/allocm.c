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
#define ALLOCM_END (-1)

/* The free list head (an index, or ALLOCM_END) */
static inline int32_t* allocm_head(struct allocm_entry* const map) {
  return &map[0].value;
}

/* Block i's entry: next free index, ALLOCM_END, or RCSW_ALLOCM_INUSE */
static inline int32_t* allocm_block(struct allocm_entry* const map, size_t i) {
  return &map[i + 1].value;
}

void allocm_init(struct allocm_entry* map, size_t n_blocks) {
  /* chain all blocks in index order: 0 -> 1 -> ... -> n-1 -> end */
  for (size_t i = 0; i < n_blocks; ++i) {
    *allocm_block(map, i) = (i + 1 < n_blocks) ? (int32_t)(i + 1) : ALLOCM_END;
  }
  *allocm_head(map) = (n_blocks > 0) ? 0 : ALLOCM_END;
} /* allocm_init() */

int allocm_alloc(struct allocm_entry* map) {
  int32_t idx = *allocm_head(map);
  if (ALLOCM_END == idx) {
    return -1;
  }
  *allocm_head(map)               = *allocm_block(map, (size_t)idx);
  *allocm_block(map, (size_t)idx) = RCSW_ALLOCM_INUSE;
  return idx;
} /* allocm_alloc() */

void allocm_free(struct allocm_entry* map, size_t index) {
  if (RCSW_ALLOCM_INUSE != *allocm_block(map, index)) {
    return; /* already free: pushing it again would create a cycle */
  }
  *allocm_block(map, index) = *allocm_head(map);
  *allocm_head(map)         = (int32_t)index;
} /* allocm_free() */

END_C_DECLS
