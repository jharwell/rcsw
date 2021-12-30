/**
 * @file bstree.c
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
#include "rcsw/ds/bstree.h"
#include <limits.h>
#include <stdlib.h>
#include "rcsw/common/dbg.h"
#include "rcsw/ds/bstree_node.h"
#include "rcsw/ds/int_tree.h"
#include "rcsw/ds/ostree_node.h"
#include "rcsw/ds/rbtree.h"
#include "rcsw/utils/utils.h"

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
#define MODULE_ID M_DS_BSTREE

/*******************************************************************************
 * Forward Declarations
 ******************************************************************************/
BEGIN_C_DECLS

/*******************************************************************************
 * API Functions
 ******************************************************************************/
struct bstree* bstree_init_internal(struct bstree* tree_in,
                                    const struct ds_params* const params,
                                    size_t node_size) {
  RCSW_FPC_NV(NULL,
            params != NULL,
            params->tag == DS_BSTREE,
            params->cmpe != NULL,
            params->el_size > 0);

  struct bstree* tree = NULL;
  int i;
  if (params->flags & DS_APP_DOMAIN_HANDLE) {
    RCSW_CHECK_PTR(tree_in);
    tree = tree_in;
  } else {
    tree = malloc(sizeof(struct bstree));
    RCSW_CHECK_PTR(tree);
  }
  tree->flags = params->flags;
  tree->root = NULL;
  tree->nil = NULL;

  if (params->flags & DS_APP_DOMAIN_NODES) {
    RCSW_CHECK_PTR(params->nodes);
    SOFT_ASSERT(
        params->max_elts != -1,
        "ERROR: Cannot have uncapped tree size with DS_APP_DOMAIN_NODES");

    /*
     * Initialize free list of bstree_nodes. The bstree requires 2 internal
     * nodes for root and nil, hence the +2.
     */
    for (i = 0; i < params->max_elts + 2; ++i) {
      ((int*)(params->nodes))[i] = -1;
    }
    tree->nodes = params->nodes;
  }

  if (params->flags & DS_APP_DOMAIN_DATA) {
    RCSW_CHECK_PTR(params->elements);
    SOFT_ASSERT(
        params->max_elts != -1,
        "ERROR: Cannot have uncapped tree size with DS_APP_DOMAIN_DATA");

    /*
     * Initialize free list of bstree_nodes. The bstree requires 2 internal
     * nodes for root and nil, hence the +2.
     */
    for (i = 0; i < params->max_elts + 2; ++i) {
      ((int*)(params->elements))[i] = -1;
    }
    tree->elements = params->elements;
  }

  tree->cmpe = params->cmpe;
  tree->current = 0;
  tree->printe = params->printe;
  tree->max_elts = params->max_elts;
  tree->el_size = params->el_size;

  tree->nil = bstree_node_create(tree, NULL, NULL, NULL, node_size);
  RCSW_CHECK_PTR(tree->nil);
  tree->nil->parent = tree->nil->left = tree->nil->right = tree->nil;
  tree->nil->red = FALSE;

  tree->root = bstree_node_create(tree, NULL, NULL, NULL, node_size);
  RCSW_CHECK_PTR(tree->root);
  tree->root->parent = tree->root->left = tree->root->right = tree->nil;
  tree->root->red = FALSE;

  if (tree->flags & DS_BSTREE_INTERVAL) {
    int_tree_init_helper(tree);
  } else if (tree->flags & DS_BSTREE_OS) {
    ostree_init_helper(tree);
  }
  DBGD("max_elts=%d el_size=%zu flags=0x%08x\n",
       tree->max_elts,
       tree->el_size,
       tree->flags);
  return tree;

error:
  bstree_destroy(tree);
  errno = EAGAIN;
  return NULL;
} /* bstree_init_internal() */

void bstree_destroy(struct bstree* tree) {
  RCSW_FPC_V(NULL != tree);

  if (tree->root != NULL) {
    bstree_traverse_nodes_postorder(tree, tree->root, bstree_node_destroy);
  }

  /*
   * Special case to delete nil sentinel node (not reachable from the rest of
   * the tree via traversal)
   */
  bstree_node_destroy(tree, tree->nil);

  if (!(tree->flags & DS_APP_DOMAIN_HANDLE)) {
    free(tree);
  }
} /* bstree_destroy() */

void* bstree_data_query(const struct bstree* const tree, const void* const key) {
  RCSW_FPC_NV(NULL, tree != NULL, key != NULL);

  struct bstree_node* node = bstree_node_query(tree, tree->root->left, key);
  return (node == NULL) ? NULL : node->data;
} /* bstree_data_query() */

struct bstree_node* bstree_node_query(const struct bstree* const tree,
                                      struct bstree_node* const search_root,
                                      const void* const key) {
  struct bstree_node* x = search_root;
  while (x != tree->nil) {
    int res;
    if ((res = tree->cmpe(key, x->key)) == 0) {
      return x;
    }
    x = res < 0 ? x->left : x->right;
  } /* while() */
  return NULL;
} /* bstree_node_query() */

int bstree_traverse(struct bstree* const tree,
                    int (*cb)(const struct bstree* const tree,
                              struct bstree_node* const node),
                    enum bstree_traversal_type type) {
  RCSW_FPC_NV(ERROR, tree != NULL, cb != NULL);

  if (BSTREE_TRAVERSE_PREORDER == type) {
    return bstree_traverse_nodes_preorder(tree, tree->root->left, cb);
  } else if (BSTREE_TRAVERSE_INORDER == type) {
    return bstree_traverse_nodes_inorder(tree, tree->root->left, cb);
  } else if (BSTREE_TRAVERSE_POSTORDER == type) {
    return bstree_traverse_nodes_postorder(tree, tree->root->left, cb);
  }
  return -1;
} /* bstree_traverse() */

status_t bstree_insert_internal(struct bstree* const tree,
                                void* const key,
                                void* const data,
                                size_t node_size) {
  RCSW_FPC_NV(ERROR, tree != NULL, key != NULL, data != NULL);

  struct bstree_node* node = tree->root->left;
  struct bstree_node* parent = tree->root;
  int res;

  /* Find correct insertion point */
  while (node != tree->nil) {
    parent = node;

    /* no duplicates allowed */
    if ((res = tree->cmpe(key, node->key)) == 0) {
      return ERROR;
    }
    node = res < 0 ? node->left : node->right;
  } /* while() */

  /*
   * Create node from key and data, and link into tree hierarchy
   */
  node = bstree_node_create(tree, parent, key, data, node_size);
  RCSW_CHECK_PTR(node);
  if (parent == tree->root || tree->cmpe(key, parent->key) < 0) {
    parent->left = node;
  } else {
    parent->right = node;
  }

  if (tree->flags & DS_BSTREE_REDBLACK) {
    /*
     * Fixup interval tree/OS-Tree auxiliary field. Must be done BEFORE
     * fixing up tree red-black tree structure, to update the fields of
     * nodes from the inserted position up to the root. Because the
     * red-black fixup process will cause at most 3 rotations, simply fixing
     * up the auxiliary field during rotations is not enough.
     */
    if (tree->flags & DS_BSTREE_INTERVAL) {
      int_tree_high_fixup(tree, (struct int_tree_node*)node);
    } else if (tree->flags & DS_BSTREE_OS) {
      ostree_count_fixup(tree, (struct ostree_node*)node, OSTREE_FIXUP_INSERT);
    }

    node->red = TRUE;

    /*
     * Fixup tree structure in the event that it was wrecked by the insert
     */
    rbtree_insert_fixup(tree, node);

    tree->root->left->red = FALSE; /* first node is always black */

    /* Verify properties of RB Tree still hold */
    RCSW_FPC_NV(ERROR, !tree->root->red);
    RCSW_FPC_NV(ERROR, !tree->nil->red);
    RCSW_FPC_NV(ERROR,
              rbtree_node_black_height(tree->root->left->left) ==
                  rbtree_node_black_height(tree->root->left->right));
  }
  tree->current++;

  return OK;

error:
  return ERROR;
} /* bstree_insert_internal() */

status_t bstree_remove(struct bstree* const tree, const void* const key) {
  RCSW_FPC_NV(ERROR, tree != NULL, key != NULL);

  struct bstree_node* victim = bstree_node_query(tree, tree->root->left, key);
  RCSW_CHECK_PTR(victim);
  return bstree_delete(tree, victim, NULL);

error:
  return ERROR;
} /* bstree_remove() */

status_t bstree_delete(struct bstree* const tree,
                       struct bstree_node* z,
                       void* const e) /* to be filled if non-NULL */
{
  RCSW_FPC_NV(ERROR, tree != NULL, z != NULL);

  struct bstree_node* x;
  struct bstree_node* y;

  /*
   * Locate the parent or succesor of the node to delete
   */
  if (z->left == tree->nil || z->right == tree->nil) {
    y = z;
  } else {
    y = bstree_node_successor(tree, z);
  }
  x = (y->left == tree->nil) ? y->right : y->left;

  /*
   * Unlink the victim node
   */
  if ((x->parent = y->parent) == tree->root) {
    tree->root->left = x;
  } else {
    if (y == y->parent->left) {
      y->parent->left = x;
    } else {
      y->parent->right = x;
    }
  }

  /*
   * Fix up RBTree structure if required
   */
  if (tree->flags & DS_BSTREE_REDBLACK && y->red == FALSE) {
    /*
     * Fixup interval tree/OS-Tree auxiliary field. Must be done BEFORE
     * fixing up tree red-black tree structure, to update the fields of
     * nodes from the inserted position up to the root. Because the
     * red-black fixup process will cause at most 3 rotations, simply fixing
     * up the auxiliary field during rotations is not enough.
     */
    if (tree->flags & DS_BSTREE_INTERVAL) {
      int_tree_high_fixup(tree, (struct int_tree_node*)x);
    } else if (tree->flags & DS_BSTREE_OS) {
      ostree_count_fixup(tree, (struct ostree_node*)x, OSTREE_FIXUP_DELETE);
    }

    rbtree_delete_fixup(tree, x);
  }

  if (y != z) {
    y->left = z->left;
    y->right = z->right;
    y->parent = z->parent;
    y->red = z->red;
    z->left->parent = z->right->parent = y;
    if (z == z->parent->left) {
      z->parent->left = y;
    } else {
      z->parent->right = y;
    }
  }

  if (tree->flags & DS_BSTREE_REDBLACK) {
    /* Verify properties of RB Tree still hold */
    RCSW_FPC_NV(ERROR, !tree->root->red);
    RCSW_FPC_NV(ERROR, !tree->nil->red);
    RCSW_FPC_NV(ERROR,
              rbtree_node_black_height(tree->root->left->left) ==
                  rbtree_node_black_height(tree->root->left->right));
  }
  if (NULL != e) {
    ds_elt_copy(e, z->data, tree->el_size);
  }
  bstree_node_destroy(tree, z);
  tree->current--;
  return OK;
} /* bstree_delete() */

void bstree_print(struct bstree* const tree) {
  if (NULL == tree) {
    DPRINTF("BSTREE: < NULL tree >\n");
    return;
  } else if (bstree_isempty(tree)) {
    DPRINTF("BSTREE: < Empty tree >\n");
    return;
  } else if (tree->printe == NULL) {
    DPRINTF("BSTREE: < No print function >\n");
    return;
  }

  bstree_traverse_nodes_inorder(tree,
                                tree->root,
                                (int (*)(const struct bstree* const,
                                         struct bstree_node*))bstree_node_print);
} /* bstree_print() */

END_C_DECLS
