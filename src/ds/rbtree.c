/**
 * \file rbtree.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/rbtree.h"

#include "rcsw/common/fpc.h"
#include "rcsw/ds/bstree.h"
#include "rcsw/ds/bstree_node.h"
#include "rcsw/er/client.h"

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

void rbtree_insert_fixup(struct bstree* const tree, struct bstree_node* node) {
  /*
   * If the parent node is black we are all set, if it is red we have
   * the following possible cases to deal with.  We iterate through
   * the rest of the tree to make sure none of the required properties
   * is violated.
   *
   *  1. The uncle is red. We repaint both the parent and uncle black and
   *     repaint the grandparent node red.
   *
   *  2. The uncle is black and the new node is the right child of its parent,
   *     and the parent in turn is the left child of its parent.  We do a left
   *     rotation to switch the roles of the parent and child, relying on
   *     further iterations to fixup the old parent.
   *
   *  3. The uncle is black and the new node is the left child of its parent,
   *     and the parent in turn is the left child of its parent.  We switch the
   *     colors of the parent and grandparent and perform a right rotation
   *     around the grandparent.  This makes the former parent the parent of the
   *     new node and the former grandparent.
   *
   * Note that because we use a sentinel for the root node we never need to
   * worry about replacing the root.
   */
  while (node->parent->red) {
    struct bstree_node* uncle;
    if (node->parent == node->parent->parent->left) {
      uncle = node->parent->parent->right;
      if (uncle->red) {
        node->parent->red = false;
        uncle->red = false;
        node->parent->parent->red = true;
        node = node->parent->parent;
      } else /* if (uncle->color == black) */ {
        if (node == node->parent->right) {
          node = node->parent;
          bstree_node_rotate_left(tree, node);
        }
        node->parent->red = false;
        node->parent->parent->red = true;
        bstree_node_rotate_right(tree, node->parent->parent);
      }
    } else { /* if (node->parent == node->parent->parent->right) */
      uncle = node->parent->parent->left;
      if (uncle->red == true) {
        node->parent->red = false;
        uncle->red = false;
        node->parent->parent->red = true;
        node = node->parent->parent;
      } else /* if (uncle->color == black) */ {
        if (node == node->parent->left) {
          node = node->parent;
          bstree_node_rotate_right(tree, node);
        }
        node->parent->red = false;
        node->parent->parent->red = true;
        bstree_node_rotate_left(tree, node->parent->parent);
      }
    }
  } /* while() */
} /* rbtree_insert_fixup() */

void rbtree_delete_fixup(struct bstree* const tree, struct bstree_node* node) {
  struct bstree_node* sibling;

  while (node->red == 0) {
    if (node == node->parent->left) {
      sibling = node->parent->right;
      if (sibling->red == true) {
        sibling->red = false;
        node->parent->red = true;
        bstree_node_rotate_left(tree, node->parent);
        sibling = node->parent->right;
      }
      if (sibling->right->red == false && sibling->left->red == false) {
        sibling->red = true;
        node = node->parent;
      } else {
        if (sibling->right->red == false) {
          sibling->left->red = false;
          sibling->red = true;
          bstree_node_rotate_right(tree, sibling);
          sibling = node->parent->right;
        }
        sibling->red = node->parent->red;
        node->parent->red = false;
        sibling->right->red = false;
        bstree_node_rotate_left(tree, node->parent);
        break;
      }
    } else { /* if (node == node->parent->right) */
      sibling = node->parent->left;
      if (sibling->red == true) {
        sibling->red = false;
        node->parent->red = true;
        bstree_node_rotate_right(tree, node->parent);
        sibling = node->parent->left;
      }
      if (sibling->right->red == false && sibling->left->red == false) {
        sibling->red = true;
        node = node->parent;
      } else {
        if (sibling->left->red == false) {
          sibling->right->red = false;
          sibling->red = true;
          bstree_node_rotate_left(tree, sibling);
          sibling = node->parent->left;
        }
        sibling->red = node->parent->red;
        node->parent->red = false;
        sibling->left->red = false;
        bstree_node_rotate_right(tree, node->parent);
        break;
      }
    }
  } /* while() */
  node->red = false;
} /* rbtree_delete_fixup() */

int rbtree_node_black_height(const struct bstree_node* const node) {
  /*
   * Sentinel to detect when we have fallen off the tree
   */
  if (node->left == node || node->right == node) {
    return 0;
  }

  /*
   * We can pick either L or R subtree; both have the same # of nodes. I
   * arbitrarily choose L.
   */
  return rbtree_node_black_height(node->left) + (node->red == 0);
} /* rbtree_node_black_height() */

END_C_DECLS
