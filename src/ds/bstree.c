/**
 * \file
 *
 * \copyright 2017 John Harwell
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/bstree.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#define RCSW_ER_MODNAME RCSW_ER_MODNAME_BUILDER("rcsw", "ds", "bstree")
#define RCSW_ER_MODID LOG4CL_DS_BSTREE
#include "ds/inttree_node.h"
#include "ds/ostree_node.h"
#include "rcsw/core/alloc.h"
#include "rcsw/ds/bstree_node.h"
#include "rcsw/ds/inttree.h"
#include "rcsw/ds/rbtree.h"
#include "rcsw/er/client.h"

/*******************************************************************************
 * Private API
 ******************************************************************************/
BEGIN_C_DECLS

struct bstree* bstree_init_internal(struct bstree*                    tree_in,
                                    const struct bstree_config* const params,
                                    size_t                            node_size) {
  RCSW_FPC_NV(NULL, params != NULL, params->cmpkey != NULL, params->elt_size > 0);
  RCSW_ER_MODULE_INIT();

  struct bstree* tree =
    rcsw_alloc(tree_in,
               sizeof(struct bstree),
               params->flags & (RCSW_NOALLOC_HANDLE | RCSW_ZALLOC));
  RCSW_CHECK_PTR(tree);

  tree->flags = params->flags;
  tree->root  = NULL;
  tree->nil   = NULL;

  if (params->flags & RCSW_NOALLOC_META) {
    RCSW_CHECK_PTR(params->meta);
    ER_CHECK(params->max_elts != -1,
             "Cannot have uncapped tree size with "
             "RCSW_NOALLOC_META");

    /*
     * Initialize free list of bstree_nodes. The bstree requires 2 internal
     * nodes for root and nil, hence the +2.
     */
    tree->space.node_map = (struct allocm_entry*)params->meta;
    allocm_init(tree->space.node_map, (size_t)(params->max_elts + 2UL));
    tree->space.nodes =
      (struct bstree_node*)((uint8_t*)tree->space.node_map +
                            allocm_map_bytes((size_t)params->max_elts + 2));
  }

  if (params->flags & RCSW_NOALLOC_DATA) {
    RCSW_CHECK_PTR(params->elements);
    ER_CHECK(params->max_elts != -1,
             "Cannot have uncapped tree size with "
             "RCSW_NOALLOC_DATA");

    /*
     * Initialize free list of bstree_nodes. The bstree requires 2 internal
     * nodes for root and nil, hence the +2.
     */
    tree->space.db_map = (struct allocm_entry*)params->elements;
    allocm_init(tree->space.db_map, (size_t)(params->max_elts + 2UL));
    tree->space.datablocks =
      (dptr_t*)((uint8_t*)tree->space.db_map +
                allocm_map_bytes((size_t)params->max_elts + 2));
  }

  tree->cmpkey   = params->cmpkey;
  tree->current  = 0;
  tree->printe   = params->printe;
  tree->max_elts = params->max_elts;
  tree->elt_size = params->elt_size;

  tree->nil = bstree_node_create(tree, NULL, NULL, NULL, node_size);
  RCSW_CHECK_PTR(tree->nil);
  tree->nil->parent = tree->nil->left = tree->nil->right = tree->nil;
  tree->nil->red                                         = false;

  tree->root = bstree_node_create(tree, NULL, NULL, NULL, node_size);
  RCSW_CHECK_PTR(tree->root);
  tree->root->parent = tree->root->left = tree->root->right = tree->nil;
  tree->root->red                                           = false;

  if (tree->flags & RCSW_DS_BSTREE_INT) {
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

/**
 * \brief Recompute the augmented field (interval max_high or order-statistic
 * count) of \p node and each of its ancestors, from their children.
 *
 * Each field is rebuilt from its children, so the result is correct however
 * the tree was restructured.
 */
static void bstree_aux_update_path(const struct bstree* const tree,
                                   struct bstree_node*        node) {
  if (!(tree->flags & (RCSW_DS_BSTREE_INT | RCSW_DS_BSTREE_OS))) {
    return;
  }
  while (node != tree->root && node != tree->nil) {
    if (tree->flags & RCSW_DS_BSTREE_INT) {
      inttree_node_update_max((struct inttree_node*)node);
    } else {
      ostree_node_update_count((struct ostree_node*)node);
    }
    node = node->parent;
  } /* while() */
} /* bstree_aux_update_path() */

/* NOLINTNEXTLINE(readability-function-size) */
status_t bstree_insert_internal(struct bstree* const tree,
                                void* const          key,
                                void* const          data,
                                size_t               node_size) {
  RCSW_FPC_NV(ERROR, tree != NULL, key != NULL, data != NULL);

  if (bstree_isfull(tree)) {
    ER_ERR("Cannot insert: tree is full");
    errno = ENOSPC;
    return ERROR;
  }

  struct bstree_node* node   = tree->root->left;
  struct bstree_node* parent = tree->root;

  /* Find correct insertion point */
  while (node != tree->nil) {
    parent = node;

    /* no duplicates allowed */
    int res = tree->cmpkey(key, node->key);
    if (0 == res) {
      errno = EEXIST;
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

  /*
   * Fixup interval tree/OS-Tree auxiliary field along the insertion path.
   * Must be done BEFORE the red-black fixup: rotations only recompute the two
   * nodes they move, which assumes everything below them is already correct.
   */
  bstree_aux_update_path(tree, node);

  if (tree->flags & RCSW_DS_BSTREE_RB) {
    node->red = true;

    /*
     * Fixup tree structure in the event that it was wrecked by the insert
     */
    rbtree_insert_fixup(tree, node);

    tree->root->left->red = false; /* first node is always black */

    /* Verify properties of RB Tree still hold (debug builds) */
    ER_ASSERT(!tree->root->red, "Sentinel root is red");
    ER_ASSERT(!tree->nil->red, "Sentinel nil is red");
    ER_ASSERT(rbtree_node_black_height(tree->root->left->left) ==
                rbtree_node_black_height(tree->root->left->right),
              "Black heights differ");
  }
  tree->current++;

  return OK;

error:
  return ERROR;
} /* bstree_insert_internal() */

/*******************************************************************************
 * Public API
 ******************************************************************************/
struct bstree* bstree_init(struct bstree*                    tree_in,
                           const struct bstree_config* const params) {
  return bstree_init_internal(tree_in, params, sizeof(struct bstree_node));
}
status_t bstree_insert(struct bstree* tree, void* const key, void* const data) {
  return bstree_insert_internal(tree, key, data, sizeof(struct bstree_node));
}

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

  rcsw_free(tree, tree->flags & RCSW_NOALLOC_HANDLE);
} /* bstree_destroy() */

void* bstree_data_query(const struct bstree* const tree, const void* const key) {
  RCSW_FPC_NV(NULL, tree != NULL, key != NULL);

  struct bstree_node* node = bstree_node_query(tree, tree->root->left, key);
  return (node == NULL) ? NULL : node->data;
} /* bstree_data_query() */

struct bstree_node* bstree_node_query(const struct bstree* const tree,
                                      struct bstree_node* const  search_root,
                                      const void* const          key) {
  struct bstree_node* x = search_root;
  while (x != tree->nil) {
    int res = tree->cmpkey(key, x->key);
    if (0 == res) {
      return x;
    }
    x = res < 0 ? x->left : x->right;
  } /* while() */
  return NULL;
} /* bstree_node_query() */

int bstree_traverse(struct bstree* const tree,
                    int (*cb)(const struct bstree* const tree,
                              struct bstree_node* const  node),
                    enum bstree_traversal_type type) {
  RCSW_FPC_NV(ERROR, tree != NULL, cb != NULL);

  if (TRAVERSE_PREORDER == type) {
    return bstree_traverse_nodes_preorder(tree, tree->root->left, cb);
  }
  if (TRAVERSE_INORDER == type) {
    return bstree_traverse_nodes_inorder(tree, tree->root->left, cb);
  }
  if (TRAVERSE_POSTORDER == type) {
    return bstree_traverse_nodes_postorder(tree, tree->root->left, cb);
  }
  return -1;
} /* bstree_traverse() */

static void bstree_map_inorder(const struct bstree* tree,
                               struct bstree_node*  node,
                               void (*f)(void* e)) {
  if (node == tree->nil) {
    return;
  }
  bstree_map_inorder(tree, node->left, f);
  f(node->data);
  bstree_map_inorder(tree, node->right, f);
}
static void bstree_map_preorder(const struct bstree* tree,
                                struct bstree_node*  node,
                                void (*f)(void* e)) {
  if (node == tree->nil) {
    return;
  }
  f(node->data);
  bstree_map_preorder(tree, node->left, f);
  bstree_map_preorder(tree, node->right, f);
}
static void bstree_map_postorder(const struct bstree* tree,
                                 struct bstree_node*  node,
                                 void (*f)(void* e)) {
  if (node == tree->nil) {
    return;
  }
  bstree_map_postorder(tree, node->left, f);
  bstree_map_postorder(tree, node->right, f);
  f(node->data);
}
static void bstree_inject_inorder(const struct bstree* tree,
                                  struct bstree_node*  node,
                                  void (*f)(void* e, void* res),
                                  void* result) {
  if (node == tree->nil) {
    return;
  }
  bstree_inject_inorder(tree, node->left, f, result);
  f(node->data, result);
  bstree_inject_inorder(tree, node->right, f, result);
}
static void bstree_inject_preorder(const struct bstree* tree,
                                   struct bstree_node*  node,
                                   void (*f)(void* e, void* res),
                                   void* result) {
  if (node == tree->nil) {
    return;
  }
  f(node->data, result);
  bstree_inject_preorder(tree, node->left, f, result);
  bstree_inject_preorder(tree, node->right, f, result);
}
static void bstree_inject_postorder(const struct bstree* tree,
                                    struct bstree_node*  node,
                                    void (*f)(void* e, void* res),
                                    void* result) {
  if (node == tree->nil) {
    return;
  }
  bstree_inject_postorder(tree, node->left, f, result);
  bstree_inject_postorder(tree, node->right, f, result);
  f(node->data, result);
}

status_t bstree_map(struct bstree* const tree,
                    void (*f)(void* e),
                    enum bstree_traversal_type type) {
  RCSW_FPC_NV(ERROR, tree != NULL, f != NULL);
  struct bstree_node* root = tree->root->left;
  if (type == TRAVERSE_PREORDER) {
    bstree_map_preorder(tree, root, f);
  } else if (type == TRAVERSE_INORDER) {
    bstree_map_inorder(tree, root, f);
  } else if (type == TRAVERSE_POSTORDER) {
    bstree_map_postorder(tree, root, f);
  } else {
    return ERROR;
  }
  return OK;
} /* bstree_map() */

status_t bstree_inject(struct bstree* const tree,
                       void (*f)(void* e, void* result),
                       void*                      result,
                       enum bstree_traversal_type type) {
  RCSW_FPC_NV(ERROR, tree != NULL, f != NULL, result != NULL);
  struct bstree_node* root = tree->root->left;
  if (type == TRAVERSE_PREORDER) {
    bstree_inject_preorder(tree, root, f, result);
  } else if (type == TRAVERSE_INORDER) {
    bstree_inject_inorder(tree, root, f, result);
  } else if (type == TRAVERSE_POSTORDER) {
    bstree_inject_postorder(tree, root, f, result);
  } else {
    return ERROR;
  }
  return OK;
} /* bstree_inject() */

status_t bstree_remove(struct bstree* const tree, const void* const key) {
  RCSW_FPC_NV(ERROR, tree != NULL, key != NULL);

  struct bstree_node* victim = bstree_node_query(tree, tree->root->left, key);
  RCSW_CHECK_PTR(victim);
  return bstree_delete(tree, victim, NULL);

error:
  return ERROR;
} /* bstree_remove() */

/* NOLINTNEXTLINE(readability-function-size) */
status_t bstree_delete(struct bstree* const tree,
                       struct bstree_node*  victim,
                       void* const          elt) {
  RCSW_FPC_NV(ERROR, tree != NULL, victim != NULL);

  struct bstree_node* x;
  struct bstree_node* y;

  /*
   * Locate the parent or successor of the node to delete
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
  x->parent = y->parent;
  if (x->parent == tree->root) {
    tree->root->left = x;
  } else {
    if (y == y->parent->left) {
      y->parent->left = x;
    } else {
      y->parent->right = x;
    }
  }

  /*
   * y is no longer in the tree: recompute the auxiliary field from where it
   * was spliced out up to the root, BEFORE any red-black rotations (which
   * assume the subtrees they move are already correct). This must happen
   * whatever color y was.
   */
  bstree_aux_update_path(tree, x->parent);

  /*
   * Fix up RBTree structure if required
   */
  if (tree->flags & RCSW_DS_BSTREE_RB && !y->red) {
    rbtree_delete_fixup(tree, x);
  }

  if (y != victim) {
    y->left              = victim->left;
    y->right             = victim->right;
    y->parent            = victim->parent;
    y->red               = victim->red;
    victim->left->parent = victim->right->parent = y;
    if (victim == victim->parent->left) {
      victim->parent->left = y;
    } else {
      victim->parent->right = y;
    }
    /* y now roots victim's old subtree, and victim's key is gone */
    bstree_aux_update_path(tree, y);
  }

  if (tree->flags & RCSW_DS_BSTREE_RB) {
    /* Verify properties of RB Tree still hold (debug builds) */
    ER_ASSERT(!tree->root->red, "Sentinel root is red");
    ER_ASSERT(!tree->nil->red, "Sentinel nil is red");
    ER_ASSERT(rbtree_node_black_height(tree->root->left->left) ==
                rbtree_node_black_height(tree->root->left->right),
              "Black heights differ");
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
  }
  if (bstree_isempty(tree)) {
    DPRINTF(RCSW_ER_MODNAME " :  < Empty >\n");
    return;
  }
  if (tree->printe == NULL) {
    DPRINTF(RCSW_ER_MODNAME " :  < No print function >\n");
    return;
  }

  bstree_traverse_nodes_inorder(
    tree,
    RCSW_BSTREE_ROOT(tree),
    (int (*)(const struct bstree* const, struct bstree_node*))bstree_node_print);
} /* bstree_print() */

END_C_DECLS
