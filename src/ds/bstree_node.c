/**
 * \file bstree_node.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/bstree_node.h"

#include "rcsw/ds/inttree_node.h"
#include "rcsw/ds/ostree_node.h"
#include "rcsw/er/client.h"
#include "rcsw/utils/hash.h"

BEGIN_C_DECLS

/*******************************************************************************
 * API Functions
 ******************************************************************************/
struct bstree_node* bstree_node_create(const struct bstree* const tree,
                                       struct bstree_node* const parent,
                                       void* const key_in,
                                       void* const data_in,
                                       size_t node_size) {
  /* get space for the node */
  struct bstree_node* node = bstree_node_alloc(tree, node_size);
  RCSW_CHECK_PTR(node);

  /* get space for the datablock */
  node->data = bstree_node_datablock_alloc(tree);
  RCSW_CHECK_PTR(node->data);

  /*
   * Data may be NULL if creating the dummy root node
   */
  if (NULL != data_in) {
    ds_elt_copy(node->data, data_in, tree->elt_size);
  }

  /*
   * Key may be NULL if creating the dummy root node
   */
  if (NULL != key_in) {
    /* copy the key */
    memcpy(&node->key, key_in, RCSW_BSTREE_NODE_KEYSIZE);
  }

  /* create linkage */
  node->parent = parent;
  node->left = tree->nil;
  node->right = tree->nil;

  return node;

error:
  bstree_node_destroy(tree, node);
  return NULL;
} /* bstree_node_create() */

int bstree_node_destroy(const struct bstree* const tree,
                        struct bstree_node* node) {
  RCSW_FPC_NV(0, NULL != node);

  /* deallocate data block */
  bstree_node_datablock_dealloc(tree, node->data);
  node->data = NULL;

  /* deallocate node */
  bstree_node_dealloc(tree, node);
  return 0;
} /* bstree_node_destroy() */

void bstree_node_datablock_dealloc(const struct bstree* const tree, /* parent tree
                                                                     */
                                   uint8_t* datablock) { /* datablock to
                                                            deallocate */
  if (datablock == NULL) {
    return;
  }
  if (tree->flags & RCSW_NOALLOC_DATA) {
    size_t idx = (datablock - tree->space.datablocks) / tree->elt_size;

    /* mark data block as available */
    allocm_mark_free(tree->space.db_map + idx);
  } else {
    free(datablock);
  }
} /* bstree_node_datablock_dealloc() */

void* bstree_node_datablock_alloc(const struct bstree* const tree) {
  void* datablock = NULL;

  if (tree->flags & RCSW_NOALLOC_DATA) {
    /*
     * Try to find an available data block. Use hashing/linearing probing
     * instead of linear scanning. This reduces startup times if
     * initializing/building a tree with a large number of items.
     */
    /* make sure that we have 32 bits of randomness */
    uint32_t val =
        (uint32_t)(random() & 0xff) | (uint32_t)((random() & 0xff) << 8) |
        (uint32_t)((random() & 0xff) << 16) | (uint32_t)((random() & 0xff) << 24);

    size_t search_idx = hash_fnv1a(&val, 4) % ((size_t)tree->max_elts + 2);

    /*
     * The bstree requires 2 internal nodes for root and nil, hence the +2.
     */
    int alloc_idx =
        allocm_probe(tree->space.db_map, (size_t)tree->max_elts + 2, search_idx);
    RCSW_CHECK(-1 != alloc_idx);

    /* mark data block as in use */
    allocm_mark_inuse(tree->space.db_map + alloc_idx);

    datablock = tree->space.datablocks + (alloc_idx * tree->elt_size);
  } else {
    datablock = malloc(tree->elt_size);
    RCSW_CHECK_PTR(datablock);
  }

  return datablock;

error:
  return NULL;
} /* bstree_node_datablock_alloc() */

struct bstree_node* bstree_node_alloc(const struct bstree* const tree,
                                      size_t node_size) {
  struct bstree_node* node = NULL;

  if (tree->flags & RCSW_NOALLOC_META) {
    /*
     * Try to find an available data block. Use hashing/linearing probing
     * instead of linear scanning. This reduces startup times if
     * initializing/building a tree with a large number of items.
     */

    /* make sure that we have 32 bits of randomness */
    uint32_t val =
        (uint32_t)(random() & 0xff) | (uint32_t)((random() & 0xff) << 8) |
        (uint32_t)((random() & 0xff) << 16) | (uint32_t)((random() & 0xff) << 24);

    size_t search_idx = hash_fnv1a(&val, 4) % ((size_t)tree->max_elts + 2);

    /*
     * The bstree requires 2 internal nodes for root and nil, hence the +2.
     */
    int alloc_idx = allocm_probe(
        tree->space.node_map, (size_t)tree->max_elts + 2, search_idx);
    RCSW_CHECK(-1 != alloc_idx);

    /* mark node as in use */
    allocm_mark_inuse(tree->space.node_map + alloc_idx);
    node = tree->space.nodes + alloc_idx;
  }
  /* MY DOMAIN MWAHAHAHA! */
  else {
    node = malloc(node_size);
    RCSW_CHECK_PTR(node);
  }
  return node;

error:
  return NULL;
} /* bstree_node_alloc() */

void bstree_node_dealloc(const struct bstree* const tree,
                         struct bstree_node* node) {
  if (tree->flags & RCSW_NOALLOC_META) {
    ptrdiff_t idx = node - tree->space.nodes;

    /* mark node as available */
    allocm_mark_free(tree->space.node_map + idx);
  } else {
    free(node);
  }
} /* bstree_node_dealloc() */

int bstree_traverse_nodes_preorder(struct bstree* const tree,
                                   struct bstree_node* const node,
                                   int (*cb)(const struct bstree* const tree,
                                             struct bstree_node* const node)) {
  int rc = cb(tree, node);
  RCSW_CHECK(rc == 0);

  if (node->left != tree->nil) { /* recurse and operate on left subtree */
    rc = bstree_traverse_nodes_preorder(tree, node->left, cb);
    RCSW_CHECK(rc == 0);
  }
  if (node->right != tree->nil) { /* recurse and operate on right subtree */
    rc = bstree_traverse_nodes_preorder(tree, node->right, cb);
    RCSW_CHECK(rc == 0);
  }
  return 0;

error:
  return rc;
} /* bstree_traverse_nodes_preorder() */

int bstree_traverse_nodes_inorder(struct bstree* const tree,
                                  struct bstree_node* const node,
                                  int (*cb)(const struct bstree* const tree,
                                            struct bstree_node* const node)) {
  int rc = 0;
  if (node->left != tree->nil) { /* recurse and operate on left subtree */
    rc = bstree_traverse_nodes_inorder(tree, node->left, cb);
    RCSW_CHECK(rc == 0);
  }
  rc = cb(tree, node);
  RCSW_CHECK(rc == 0);
  if (node->right != tree->nil) { /* recurse and operate on right subtree */
    rc = bstree_traverse_nodes_inorder(tree, node->right, cb);
    RCSW_CHECK(rc == 0);
  }
  return 0;

error:
  return rc;
} /* bstree_traverse_nodes_inorder() */

int bstree_traverse_nodes_postorder(struct bstree* const tree,
                                    struct bstree_node* const node,
                                    int (*cb)(const struct bstree* const tree,
                                              struct bstree_node* const node)) {
  tree->depth++;
  int rc = 0;
  if (node->left != tree->nil) { /* recurse and operate on left subtree */
    rc = bstree_traverse_nodes_postorder(tree, node->left, cb);
    RCSW_CHECK(rc == 0);
    tree->depth--;
  }

  if (node->right != tree->nil) { /* recurse and operate on right subtree */
    rc = bstree_traverse_nodes_postorder(tree, node->right, cb);
    tree->depth--;
    RCSW_CHECK(rc == 0);
  }

  return cb(tree, node);
error:
  return rc;
} /* bstree_traverse_nodes_postorder() */

int bstree_node_print(const struct bstree* const tree, /* parent tree */
                      const struct bstree_node* const node) {
  tree->printe(node->data);
  return 0;
} /* bstree_node_print() */

void bstree_node_rotate_left(struct bstree* const tree,
                             struct bstree_node* node) {
  struct bstree_node* child;

  child = node->right; /* hold reference to node's right child */
  node->right = child->left; /* hold reference to node's left child */

  /* if left child exists, reset parent to node */
  if (child->left != tree->nil) {
    child->left->parent = node;
  }
  child->parent = node->parent;

  /* node was left child of parent; make new left child */
  if (node == node->parent->left) {
    node->parent->left = child;
  } else { /* node was right child of parent; make new right child */
    node->parent->right = child;
  }
  child->left = node;
  node->parent = child;

  /*
   * Update the auxiliary field for interval trees. This needs to be done
   * here, during the red-black fixup phase for insert/delete, in addition to
   * being done traveling from the inserted/deleted node up to the root of the
   * tree, to maintain the auxiliary fields properly for nodes that have been
   * rotated.
   */
  if (tree->flags & RCSW_DS_BSTREE_INTERVAL) {
    inttree_node_update_max((struct inttree_node*)node);
    inttree_node_update_max((struct inttree_node*)child);
  } else if (tree->flags & RCSW_DS_BSTREE_OS) {
    ostree_node_update_count((struct ostree_node*)node);
    ostree_node_update_count((struct ostree_node*)child);
  }
} /* bstree_node_rotate_left() */

void bstree_node_rotate_right(struct bstree* const tree,
                              struct bstree_node* node) {
  struct bstree_node* child;
  child = node->left; /* hold reference to node's left child */

  /* move tmp's right child to left child of node */
  node->left = child->right;

  /* if right child exists, reset parent to node */
  if (child->right != tree->nil) {
    child->right->parent = node;
  }
  child->parent = node->parent;

  /* node was left child of parent; make new left child */
  if (node == node->parent->left) {
    node->parent->left = child;
  } else { /* node was right child of parent; make new right child */
    node->parent->right = child;
  }
  child->right = node;
  node->parent = child;

  /*
   * Update the auxiliary field for interval trees. This needs to be done
   * here, during the red-black fixup phase for insert/delete, in addition to
   * being done traveling from the inserted/deleted node up to the root of the
   * tree, to maintain the auxiliary fields properly for nodes that have been
   * rotated.
   */
  if (tree->flags & RCSW_DS_BSTREE_INTERVAL) {
    inttree_node_update_max((struct inttree_node*)node);
    inttree_node_update_max((struct inttree_node*)child);
  } else if (tree->flags & RCSW_DS_BSTREE_OS) {
    ostree_node_update_count((struct ostree_node*)node);
    ostree_node_update_count((struct ostree_node*)child);
  }
} /* bstree_node_rotate_right() */

struct bstree_node* bstree_node_successor(const struct bstree* const tree,
                                          const struct bstree_node* node) {
  struct bstree_node* succ;

  if ((succ = node->right) != tree->nil) {
    while (succ->left != tree->nil) {
      succ = succ->left;
    } /* while() */
  } else {
    /* No right child, move up until we find it or hit the root */
    for (succ = node->parent; node == succ->right; succ = succ->parent) {
      node = succ;
    } /* for() */
    if (succ == tree->root) {
      succ = tree->nil;
    }
  }
  return succ;
} /* bstree_node_successor() */

size_t bstree_node_height(const struct bstree* const tree,
                          const struct bstree_node* const node) {
  /*
   * Sentinel to detect when we have fallen off the tree
   */
  if (node->parent == node || node->left == node || node->right == node) {
    return 0;
  }
  int height_l = bstree_node_height(tree, node->left);
  int height_r = bstree_node_height(tree, node->right);

  return (RCSW_MAX(height_l, height_r) + 1);
} /* bstree_node_height() */

END_C_DECLS
