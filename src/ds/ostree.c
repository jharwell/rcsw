/**
 * @file ostree.c
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
 * RCSW.  If not, see <http://www.gnu.org/licenses/
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/ostree.h"
#include <limits.h>
#include "rcsw/common/dbg.h"
#include "rcsw/common/fpc.h"
#include "rcsw/ds/bstree_node.h"
#include "rcsw/ds/ostree_node.h"

BEGIN_C_DECLS

/*******************************************************************************
 * API Functions
 ******************************************************************************/
void ostree_init_helper(struct bstree* const tree) {
  ((struct ostree_node*)tree->root)->count = 0;
  ((struct ostree_node*)tree->nil)->count = 0;
  *(int*)tree->root->data = 0xCC;
  *(int*)tree->nil->data = 0xCD;
} /* ostree_init_helper() */

struct ostree_node* ostree_select(const struct bstree* const tree,
                                  struct bstree_node* const node_in,
                                  size_t i) {
  RCSW_FPC_NV(NULL, NULL != tree, NULL != node_in, i <= tree->current - 1);

  if (node_in == tree->nil) {
    return NULL;
  }

  struct ostree_node* node = (struct ostree_node*)node_in;
  int k = node->left->count;
  if ((int)i == k) {
    return node;
  } else if ((int)i < k) {
    return ostree_select(tree, (struct bstree_node*)node->left, i);
  } else {
    return ostree_select(tree, (struct bstree_node*)node->right, i - (k + 1));
  }
} /* ostree_select() */

int ostree_rank(const struct bstree* const tree,
                const struct ostree_node* const node) {
  RCSW_FPC_NV(-1, NULL != tree, NULL != node);
  int r = node->left->count;
  const struct ostree_node* y = node;

  while (y != (struct ostree_node*)tree->root) {
    if (y == y->parent->right) {
      r += y->parent->left->count + 1;
    }
    y = y->parent;
  } /* while() */
  return r;
} /* ostree_rank() */

END_C_DECLS
