/**
 * \file bstree.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/bstree.h"

#include <limits.h>
#include <stdlib.h>

#define RCSW_ER_MODNAME "rcsw.ds.bstree"
#define RCSW_ER_MODID M_DS_BSTREE
#include "rcsw/er/client.h"

#include "rcsw/ds/bstree_node.h"
#include "rcsw/ds/inttree.h"
#include "rcsw/ds/ostree_node.h"
#include "rcsw/ds/rbtree.h"
#include "rcsw/utils/utils.h"

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
              params->tag == ekRCSW_DS_BSTREE,
              params->cmpkey != NULL,
              params->elt_size > 0);
  RCSW_ER_MODULE_INIT();

  struct bstree* tree = NULL;
  if (params->flags & RCSW_DS_NOALLOC_HANDLE) {
    tree = tree_in;
  } else {
    tree = malloc(sizeof(struct bstree));
  }
  RCSW_CHECK_PTR(tree);

  tree->flags = params->flags;
  tree->root = NULL;
  tree->nil = NULL;

  if (params->flags & RCSW_DS_NOALLOC_NODES) {
    RCSW_CHECK_PTR(params->nodes);
    ER_CHECK(params->max_elts != -1,
                  "Cannot have uncapped tree size with "
                  "RCSW_DS_NOALLOC_NODES");

    /*
     * Initialize free list of bstree_nodes. The bstree requires 2 internal
     * nodes for root and nil, hence the +2.
     */
    tree->space.node_map = (struct allocm_entry*)params->nodes;
    allocm_init(tree->space.node_map, params->max_elts + 2);
    tree->space.nodes =
        (struct bstree_node*)(tree->space.node_map + params->max_elts + 2);
  }

  if (params->flags & RCSW_DS_NOALLOC_DATA) {
    RCSW_CHECK_PTR(params->elements);
    ER_CHECK(params->max_elts != -1,
             "Cannot have uncapped tree size with "
             "RCSW_DS_NOALLOC_DATA");

    /*
     * Initialize free list of bstree_nodes. The bstree requires 2 internal
     * nodes for root and nil, hence the +2.
     */
    tree->space.db_map = (struct allocm_entry*)params->elements;
    allocm_init(tree->space.db_map, params->max_elts + 2);
    tree->space.datablocks =
        (uint8_t*)(tree->space.db_map + params->max_elts + 2);
  }

  tree->cmpkey = params->cmpkey;
  tree->current = 0;
  tree->printe = params->printe;
  tree->max_elts = params->max_elts;
  tree->elt_size = params->elt_size;

  tree->nil = bstree_node_create(tree, NULL, NULL, NULL, node_size);
  RCSW_CHECK_PTR(tree->nil);
  tree->nil->parent = tree->nil->left = tree->nil->right = tree->nil;
  tree->nil->red = false;

  tree->root = bstree_node_create(tree, NULL, NULL, NULL, node_size);
  RCSW_CHECK_PTR(tree->root);
  tree->root->parent = tree->root->left = tree->root->right = tree->nil;
  tree->root->red = false;

  if (tree->flags & RCSW_DS_BSTREE_INTERVAL) {
    inttree_init_helper(tree);
  } else if (tree->flags & RCSW_DS_BSTREE_OS) {
    ostree_init_helper(tree);
  }
  ER_DEBUG("max_elts=%d elt_size=%zu flags=0x%08x",
           tree->max_elts,
           tree->elt_size,
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

  if (!(tree->flags & RCSW_DS_NOALLOC_HANDLE)) {
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
    if ((res = tree->cmpkey(key, x->key)) == 0) {
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

  if (ekBSTREE_TRAVERSE_PREORDER == type) {
    return bstree_traverse_nodes_preorder(tree, tree->root->left, cb);
  } else if (ekBSTREE_TRAVERSE_INORDER == type) {
    return bstree_traverse_nodes_inorder(tree, tree->root->left, cb);
  } else if (ekBSTREE_TRAVERSE_POSTORDER == type) {
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
    if ((res = tree->cmpkey(key, node->key)) == 0) {
      return ERROR;
    }
    node = res < 0 ? node->left : node->right;
  } /* while() */

  /*
   * Create node from key and data, and link into tree hierarchy
   */
  node = bstree_node_create(tree, parent, key, data, node_size);
  RCSW_CHECK_PTR(node);
  if (parent == tree->root || tree->cmpkey(key, parent->key) < 0) {
    parent->left = node;
  } else {
    parent->right = node;
  }

  if (tree->flags & RCSW_DS_BSTREE_RB) {
    /*
     * Fixup interval tree/OS-Tree auxiliary field. Must be done BEFORE
     * fixing up tree red-black tree structure, to update the fields of
     * nodes from the inserted position up to the root. Because the
     * red-black fixup process will cause at most 3 rotations, simply fixing
     * up the auxiliary field during rotations is not enough.
     */
    if (tree->flags & RCSW_DS_BSTREE_INTERVAL) {
      inttree_high_fixup(tree, (struct inttree_node*)node);
    } else if (tree->flags & RCSW_DS_BSTREE_OS) {
      ostree_count_fixup(tree, (struct ostree_node*)node, ekOSTREE_FIXUP_INSERT);
    }

    node->red = true;

    /*
     * Fixup tree structure in the event that it was wrecked by the insert
     */
    rbtree_insert_fixup(tree, node);

    tree->root->left->red = false; /* first node is always black */

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
                       struct bstree_node* victim,
                       void* const elt) {
  RCSW_FPC_NV(ERROR, tree != NULL, victim != NULL);

  struct bstree_node* x;
  struct bstree_node* y;

  /*
   * Locate the parent or succesor of the node to delete
   */
  if (victim->left == tree->nil || victim->right == tree->nil) {
    y = victim;
  } else {
    y = bstree_node_successor(tree, victim);
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
  if (tree->flags & RCSW_DS_BSTREE_RB && y->red == false) {
    /*
     * Fixup interval tree/OS-Tree auxiliary field. Must be done BEFORE
     * fixing up tree red-black tree structure, to update the fields of
     * nodes from the inserted position up to the root. Because the
     * red-black fixup process will cause at most 3 rotations, simply fixing
     * up the auxiliary field during rotations is not enough.
     */
    if (tree->flags & RCSW_DS_BSTREE_INTERVAL) {
      inttree_high_fixup(tree, (struct inttree_node*)x);
    } else if (tree->flags & RCSW_DS_BSTREE_OS) {
      ostree_count_fixup(tree, (struct ostree_node*)x, ekOSTREE_FIXUP_DELETE);
    }

    rbtree_delete_fixup(tree, x);
  }

  if (y != victim) {
    y->left = victim->left;
    y->right = victim->right;
    y->parent = victim->parent;
    y->red = victim->red;
    victim->left->parent = victim->right->parent = y;
    if (victim == victim->parent->left) {
      victim->parent->left = y;
    } else {
      victim->parent->right = y;
    }
  }

  if (tree->flags & RCSW_DS_BSTREE_RB) {
    /* Verify properties of RB Tree still hold */
    RCSW_FPC_NV(ERROR, !tree->root->red);
    RCSW_FPC_NV(ERROR, !tree->nil->red);
    RCSW_FPC_NV(ERROR,
                rbtree_node_black_height(tree->root->left->left) ==
                    rbtree_node_black_height(tree->root->left->right));
  }
  if (NULL != elt) {
    ds_elt_copy(elt, victim->data, tree->elt_size);
  }
  bstree_node_destroy(tree, victim);
  tree->current--;
  return OK;
} /* bstree_delete() */

void bstree_print(struct bstree* const tree) {
  if (NULL == tree) {
    DPRINTF(RCSW_ER_MODNAME " :  < NULL >\n");
    return;
  } else if (bstree_isempty(tree)) {
    DPRINTF(RCSW_ER_MODNAME " :  < Empty >\n");
    return;
  } else if (tree->printe == NULL) {
    DPRINTF(RCSW_ER_MODNAME " :  < No print function >\n");
    return;
  }

  bstree_traverse_nodes_inorder(tree,
                                RCSW_BSTREE_ROOT(tree),
                                (int (*)(const struct bstree* const,
                                         struct bstree_node*))bstree_node_print);
} /* bstree_print() */

END_C_DECLS
