/**
 * \file
 *
 * \copyright 2023 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * \ingroup ds
 *
 * \brief Allocation maps for fixed-size blocks in caller-provided memory.
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stddef.h>
#include <stdint.h>

#include "rcsw/al/types.h"
#include "rcsw/core/compilers.h"

/*******************************************************************************
 * Types
 ******************************************************************************/
BEGIN_C_DECLS
/**
 * \brief An entry in an allocation map for fixed-size blocks (datablocks or
 * nodes) in caller-provided memory.
 *
 * A map managing N blocks is N+1 entries long (see \ref allocm_n_entries()):
 * entry 0 holds the index of the first free block (or -1 if none), and entry
 * i+1 describes block i: the index of the next free block, -1 for the end of
 * the free list, or \ref RCSW_ALLOCM_INUSE. Allocation and deallocation are
 * O(1).
 */
struct RCSW_ATTR(packed, aligned(sizeof(dptr_t))) allocm_entry {
  int32_t value;
};

/** \brief Marks a block as allocated in an allocation map. */
#define RCSW_ALLOCM_INUSE (-2)

/*******************************************************************************
 * Public API
 ******************************************************************************/
/**
 * \brief The # of \ref allocm_entry needed to manage \p n_blocks blocks.
 */
static inline size_t allocm_n_entries(size_t n_blocks) { return n_blocks + 1; }

/**
 * \brief The # of bytes an allocation map for \p n_blocks blocks occupies.
 *
 * Each entry is \ref dptr_t aligned and sized, so storage laid out after the
 * map keeps \ref RCSW_CONFIG_PTR_ALIGN alignment.
 */
static inline size_t allocm_map_bytes(size_t n_blocks) {
  return allocm_n_entries(n_blocks) * sizeof(struct allocm_entry);
}

/**
 * \brief Whether block \p index is currently allocated.
 */
static inline bool_t allocm_inuse(const struct allocm_entry* map, size_t index) {
  return RCSW_ALLOCM_INUSE == map[index + 1].value;
}

/**
 * \brief Initialize an allocation map, marking all \p n_blocks blocks as free.
 *
 * \p map must have room for \ref allocm_map_bytes() bytes.
 */
RCSW_LOCAL void allocm_init(struct allocm_entry* map, size_t n_blocks);

/**
 * \brief Allocate a free block.
 *
 * \return Index of the allocated block, or -1 if all blocks are in use.
 */
RCSW_LOCAL int allocm_alloc(struct allocm_entry* map);

/**
 * \brief Return a block to the free list.
 *
 * Freeing a block which is not currently allocated has no effect.
 */
RCSW_LOCAL void allocm_free(struct allocm_entry* map, size_t index);

END_C_DECLS
