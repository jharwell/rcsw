/**
 * \file allocm.h
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/rcsw.h"

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/
/**
 * \brief An entry in an allocation map for datablocks.
 */
struct RCSW_ATTR(packed, aligned (sizeof(dptr_t))) allocm_entry {
  int32_t value;
};

/*******************************************************************************
 * API Functions
 ******************************************************************************/
/**
 * \brief Mark an entry in the datablock map as available.
 */
static inline void allocm_mark_free(struct allocm_entry* entry) {
  entry->value = -1;
}

/**
 * \brief Mark an etry in the datablock mark as in use.
 */
static inline void allocm_mark_inuse(struct allocm_entry* entry) {
  entry->value = 0;
}

/**
 * \brief Search a metadata area for a free datablock/node
 *
 * \param map Pointer to the start of elements/nodes metadata block
 *
 * \param max_elts Max # of elements for the data structure (i.e. the size of
 *                 the metadata area in units)
 *
 * \param index Index to start probing at.
 *
 * \return Index of free datablock in map, or -1 if none found
 **/
int allocm_probe(struct allocm_entry *map,
                 size_t max_elts,
                 size_t index) RCSW_PURE;

/**
 * \brief Initialize a datablock allocation map, marking each block as available.
 */
void allocm_init(struct allocm_entry* map, size_t max_elts);
