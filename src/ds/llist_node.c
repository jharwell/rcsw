/**
 * \file llist_node.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/llist_node.h"

#define RCSW_ER_MODNAME "rcsw.ds.llist"
#define RCSW_ER_MODID M_DS_LLIST
#include "rcsw/common/fpc.h"
#include "rcsw/er/client.h"

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

struct llist_node* llist_node_alloc(struct llist* const list) {
  struct llist_node* node = NULL;
  if (list->flags & RCSW_NOALLOC_META) {
    /*
     * Try to find an available data block. Start searching at the index
     * corresponding to the element after that current # of elements in the
     * list--this makes the search process O(1) even for large lists.
     */
    int alloc_idx =
        allocm_probe(list->space.node_map, (size_t)list->max_elts, list->current);
    RCSW_CHECK(-1 != alloc_idx);
    node = list->space.nodes + alloc_idx;

    /* mark node as in use */
    allocm_mark_inuse(list->space.node_map + alloc_idx);

    ER_TRACE("Allocated llist_node %zu/%d", list->current + 1, list->max_elts);
  } else {
    node = calloc(1, sizeof(struct llist_node));
    RCSW_CHECK_PTR(node);
  }

  return node;

error:
  return NULL;
} /* llist_node_alloc() */

void llist_node_dealloc(struct llist* const list, struct llist_node* node) {
  if (list->flags & RCSW_NOALLOC_META) {
    int index = node - list->space.nodes;

    allocm_mark_free(list->space.node_map + index);

    ER_TRACE("Deallocated llist_node %d/%d", index + 1, list->max_elts);
  } else {
    free(node);
  }
} /* llist_node_dealloc() */

void llist_node_destroy(struct llist* const list, struct llist_node* node) {
  RCSW_FPC_V(NULL != node);

  /* deallocate data block, first, then llist_node */
  llist_node_datablock_dealloc(list, node->data);
  llist_node_dealloc(list, node);
} /* llist_node_destroy() */

struct llist_node* llist_node_create(struct llist* const list,
                                     void* const data_in) {
  /* get space for llist_node */
  struct llist_node* node = llist_node_alloc(list);
  RCSW_CHECK_PTR(node);

  /*
   * Get space for the datablock and copy the data, unless DS_LLIST_DB_DISOWN or
   * DS_LLIST_NO_DATA was passed
   */
  if (list->flags & (RCSW_DS_LLIST_DB_DISOWN | RCSW_DS_LLIST_DB_PTR)) {
    node->data = data_in;
  } else {
    node->data = llist_node_datablock_alloc(list);
    RCSW_CHECK_PTR(node->data);
    ds_elt_copy(node->data, data_in, list->elt_size);
  }
  return node;

error:
  llist_node_destroy(list, node);
  errno = ENOMEM;
  return NULL;
} /* llist_node_create */

/*******************************************************************************
 * Datablock Functions
 ******************************************************************************/
void llist_node_datablock_dealloc(struct llist* const list, uint8_t* datablock) {
  /* nothing to do */
  if (datablock == NULL) {
    return;
  }

  /* don't deallocate: we never allocated! */
  if (list->flags & (RCSW_DS_LLIST_DB_DISOWN | RCSW_DS_LLIST_DB_PTR)) {
    return;
  }

  if (list->flags & RCSW_NOALLOC_DATA) {
    size_t block_idx = (datablock - list->space.datablocks) / list->elt_size;

    /* mark data block as available */
    allocm_mark_free(list->space.db_map + block_idx);

    ER_TRACE("Dellocated data block %zu/%d", block_idx, list->max_elts);
  } else {
    free(datablock);
  }
} /* llist_node_datablock_dealloc() */

void* llist_node_datablock_alloc(struct llist* const list) {
  void* datablock = NULL;

  if (list->flags & RCSW_NOALLOC_DATA) {
    /*
     * Try to find an available data block. Start searching at the index
     * corresponding to the element after that current # of elements in the
     * list--this makes the search process O(1) even for large lists.
     */
    int alloc_idx =
        allocm_probe(list->space.db_map, (size_t)list->max_elts, list->current);
    RCSW_CHECK(-1 != alloc_idx);
    datablock = list->space.datablocks + alloc_idx * list->elt_size;

    /* mark data block as inuse */
    allocm_mark_inuse(list->space.db_map + alloc_idx);

    ER_TRACE("Allocated data block %d/%d", alloc_idx, list->max_elts);
  } else {
    datablock = malloc(list->elt_size);
    RCSW_CHECK_PTR(datablock);
  }

  return datablock;

error:
  return NULL;
} /* llist_node_datablock_alloc() */

END_C_DECLS
