/**
 * \file ostree_node.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/ostree_node.h"

BEGIN_C_DECLS

/*******************************************************************************
 * API Functions
 ******************************************************************************/
void ostree_node_update_count(struct ostree_node* const node) {
  /*
   * Need to make sure not to change the count of tree->nil, as select()
   * relies on that being 0.
   */
  if (node->parent != node) {
    node->count = node->left->count + node->right->count + 1;
  }
} /* ostree_node_update_count() */

void ostree_count_fixup(const struct bstree* const tree,
                        struct ostree_node* node,
                        enum ostree_fixup_type type) {
  /*
   * Deletion: decrement the counter of the parent node by 1.
   * Insertion: No action; seamlessly handled by update_count().
   */
  if (type == OSTREE_FIXUP_DELETE) {
    node = node->parent;
    node->count--;
  }
  while (node != (struct ostree_node*)tree->root) {
    ostree_node_update_count(node);
    node = node->parent;
  } /* while() */
} /* ostree_count_fixup() */

END_C_DECLS
