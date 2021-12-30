/**
 * @file llist_node.c
 *
 * @copyright 2017 John Harwell, All rights reserved.
 *
 * This file is part of RCSW.
 *
 * RCSW is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * RCSW is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * RCSW.  If not, see <http://www.gnu.org/licenses
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/llist_node.h"
#include "rcsw/common/dbg.h"
#include "rcsw/common/fpc.h"
#include "rcsw/utils/hash.h"

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
#define MODULE_ID DS_LLIST

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

struct llist_node* llist_node_alloc(struct llist* const list) {
  struct llist_node* node = NULL;

  if (list->flags & DS_APP_DOMAIN_NODES) {
    /*
     * Try to find an available data block. Start searching at the index
     * corresponding to the element after that current # of elements in the
     * list--this makes the search process O(1) even for large lists.
     */

    size_t index = list->current;
    node = ds_meta_probe(
        list->nodes, sizeof(struct llist_node), (size_t)list->max_elts, &index);
    RCSW_CHECK_PTR(node);

    ((int*)(list->nodes))[index] = 0; /* mark node as in use */
    DBGV("Allocated llist_node %zu/%d\n", index + 1, list->max_elts);
  } else {
    node = calloc(1, sizeof(struct llist_node));
    RCSW_CHECK_PTR(node);
  }

  return node;

error:
  return NULL;
} /* llist_node_alloc() */

void llist_node_dealloc(struct llist* const list, struct llist_node* node) {
  if (list->flags & DS_APP_DOMAIN_NODES) {
    struct llist_node* nodes_start =
        (struct llist_node*)(list->nodes +
                             ds_calc_meta_space((size_t)list->max_elts));
    int index = node - nodes_start;

    ((int*)(list->nodes))[index] = -1; /* mark node as available */
    DBGV("Deallocated llist_node %d/%d\n", index + 1, list->max_elts);
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

  /* get space for the datablock and copy the data, unless
   * DS_LLIST_NO_DB or DS_LLIST_NO_DATA was passed */
  if (!(list->flags & (DS_LLIST_NO_DB | DS_LLIST_PTR_CMP))) {
    node->data = llist_node_datablock_alloc(list);
    RCSW_CHECK_PTR(node->data);
    ds_elt_copy(node->data, data_in, list->el_size);
  } else {
    node->data = data_in;
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
  if (list->flags & (DS_LLIST_NO_DB | DS_LLIST_PTR_CMP)) {
    return;
  }
  if (list->flags & DS_APP_DOMAIN_DATA) {
    uint8_t* data_start = (uint8_t*)((int*)list->elements + list->max_elts);
    size_t block_index = (size_t)(datablock - data_start) / list->el_size;
    ((int*)(list->elements))[block_index] =
        -1; /* mark data block as available */

    DBGV("Dellocated data block %zu/%d\n", block_index + 1, list->max_elts);
  } else {
    free(datablock);
  }
} /* llist_node_datablock_dealloc() */

void* llist_node_datablock_alloc(struct llist* const list) {
  void* datablock = NULL;

  if (list->flags & DS_APP_DOMAIN_DATA) {
    /*
     * Try to find an available data block. Start searching at the index
     * corresponding to the element after that current # of elements in the
     * list--this makes the search process O(1) even for large lists.
     */
    size_t index = list->current;
    datablock = ds_meta_probe(
        list->elements, list->el_size, (size_t)list->max_elts, &index);
    RCSW_CHECK_PTR(datablock);

    ((int*)(list->elements))[index] = 0; /* mark data block as in use */
    DBGV("Allocated data block %zu/%d\n", index + 1, list->max_elts);
  } else {
    datablock = malloc(list->el_size);
    RCSW_CHECK_PTR(datablock);
  }

  return datablock;

error:
  return NULL;
} /* llist_node_datablock_alloc() */

END_C_DECLS
