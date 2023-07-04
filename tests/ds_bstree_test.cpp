/**
 * \file ds_bstree_test.cpp
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License Identifier:
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#define CATCH_CONFIG_PREFIX_ALL
#include <catch.hpp>

#include "rcsw/ds/bstree_node.h"
#include "rcsw/ds/inttree_node.h"
#include "rcsw/ds/ostree_node.h"

#include "tests/ds_test.h"
#include "tests/ds_bstree_test.hpp"
#include "tests/ds_test.hpp"

/*******************************************************************************
 * Namespaces/Decls
 ******************************************************************************/
namespace th::bst {


/*******************************************************************************
 * API Functions
 ******************************************************************************/
int verify_nodes_int(const struct bstree* const tree,
                        struct inttree_node * const node) {
  const uint8_t * node_key = node->key;
  uint8_t* left_key;
  uint8_t* right_key;
  struct bstree_node* nil = tree->nil;

  /*
   * Verify auxiliary field
   */

  if (node != reinterpret_cast<inttree_node*>(nil)) {
    if (node->right != reinterpret_cast<inttree_node*>(nil)) {
      CATCH_REQUIRE(node->max_high >= node->left->max_high);
    }
    if (node->right != reinterpret_cast<inttree_node*>(nil)) {
      CATCH_REQUIRE(node->max_high <= node->right->max_high);
    }
    CATCH_REQUIRE(node->max_high >= reinterpret_cast<interval_data*>(node->data)->high);
  }
  return 0;
} /* th_verify_nodes_int() */

int verify_nodes_rb(const struct bstree* const tree,
                       struct bstree_node * const node) {
  const uint8_t * node_key = node->key;
  uint8_t *left_key;
  uint8_t *right_key;

  /*
   * Verify root and nil nodes are black (RBTree property #1)
   */
  struct bstree_node* nil = tree->nil;
  int height;
  CATCH_REQUIRE(tree->root->red == 0);
  CATCH_REQUIRE(tree->nil->red == 0);
  CATCH_REQUIRE(tree->root->parent == nil);

  /*
   * Verify children are < current node if a left child, and > current node
   * if a right child
   */
  if (node->left != nil && node->right != nil) {
    left_key = node->left->key;
    right_key = node->right->key;
    CATCH_REQUIRE(th_key_cmp(left_key, node_key) <= 0);
    CATCH_REQUIRE(th_key_cmp(right_key, node_key) > 0);
  } else if (node->left != nil) {
    left_key = node->left->key;
    CATCH_REQUIRE(th_key_cmp(left_key, node_key) <= 0);
  } else if (node->right != nil) {
    right_key = node->right->key;
    CATCH_REQUIRE(th_key_cmp(right_key, node_key) > 0);
  }
  /*
   * Verify that black height of L/R subtrees are equal (RBTree property #2)
   */
  CATCH_REQUIRE(rbtree_node_black_height(node->left) ==
                rbtree_node_black_height(node->right));

  /*
   * Verify if the node is red, both children are black (RBTree property #3)
   */
  CATCH_REQUIRE(!(node->red == 1 && node->left->red == 1));
  CATCH_REQUIRE(!(node->red == 1 && node->right->red == 1));
  return 0;
} /* th_verify_nodes_rb() */

int verify_nodes_bst(const struct bstree* const tree,
                        struct bstree_node * const node) {
  const uint8_t * node_key = node->key;
  uint8_t* left_key;
  uint8_t* right_key;
  struct bstree_node* nil = tree->nil;

  /*
   * Verify children are < current node if a left child, and > current node
   * if a right child (BSTree property #1)
   */
  if (node->left != nil && node->right != nil) {
    left_key = node->left->key;
    right_key = node->right->key;
    CATCH_REQUIRE(th_key_cmp(left_key, node_key) <= 0);
    CATCH_REQUIRE(th_key_cmp(right_key, node_key) > 0);
  } else if (node->left != nil) {
    left_key = node->left->key;
    CATCH_REQUIRE(th_key_cmp(left_key, node_key) <= 0);
  } else if (node->right != nil) {
    right_key = node->right->key;
    CATCH_REQUIRE(th_key_cmp(right_key, node_key) > 0);
  }

  /*
   * Verify height of tree is O(# nodes) (BSTree property #2)
   */
  CATCH_REQUIRE(bstree_node_height(tree, node) <= 10* bstree_size(tree));

  return 0;
} /* th_verify_nodes_bst() */

} /* namespace th::bst */
