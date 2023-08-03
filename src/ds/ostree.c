/**
 * \file ostree.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/ostree.h"

#include <limits.h>

#include "rcsw/common/fpc.h"
#include "rcsw/ds/bstree_node.h"
#include "rcsw/ds/ostree_node.h"
#include "rcsw/er/client.h"

BEGIN_C_DECLS
/*******************************************************************************
 * RCSW Private Functions
 ******************************************************************************/
struct bstree* ostree_init(struct bstree* tree_in,
                           const struct bstree_params* params) {
  return bstree_init_internal(tree_in, params, sizeof(struct ostree_node));
}

status_t ostree_insert(struct bstree* tree,
                       void* const key,
                       void* const data) {
  return bstree_insert_internal(tree, key, data, sizeof(struct ostree_node));
}

/*******************************************************************************
 * API Functions
 ******************************************************************************/
void ostree_init_helper(struct bstree* const tree) {
  ((struct ostree_node*)tree->root)->count = 0;
  ((struct ostree_node*)tree->nil)->count = 0;
  *(char*)tree->root->data = 0xCC;
  *(char*)tree->nil->data = 0xCD;
} /* ostree_init_helper() */

struct ostree_node* ostree_select(const struct bstree* const tree,
                                  struct ostree_node* const node_in,
                                  int i) {
  RCSW_FPC_NV(NULL,
              NULL != tree,
              NULL != node_in,
              !bstree_isempty(tree),
              i <= (int)bstree_size(tree) - 1);

  if (node_in == (struct ostree_node*)tree->nil) {
    return NULL;
  }

  struct ostree_node* node = (struct ostree_node*)node_in;
  int k = node->left->count;
  if (i == k) {
    return node;
  } else if (i < k) {
    return ostree_select(tree, node->left, i);
  } else {
    return ostree_select(tree, node->right, i - (k + 1));
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
