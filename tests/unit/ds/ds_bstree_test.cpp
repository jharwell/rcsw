/**
 * \file
 *
 * \copyright 2023 John Harwell
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#define CATCH_CONFIG_PREFIX_ALL
#include "tests/unit/ds/ds_bstree_test.hpp"

#include <catch2/catch_test_macros.hpp>

#include "rcsw/ds/bstree_node.h"
#include "ds/inttree_node.h"
#include "ds/ostree_node.h"
#include "tests/unit/ds/ds_test.hpp"

/*******************************************************************************
 * Namespaces/Decls
 ******************************************************************************/
namespace th::bst {

std::vector<int32_t> g_inorder;

/*******************************************************************************
 * API Functions
 ******************************************************************************/
int verify_nodes_int(const struct bstree* const tree,
                     struct inttree_node* const node) {
  uint8_t*            left_key;
  uint8_t*            right_key;
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
    CATCH_REQUIRE(node->max_high >=
                  reinterpret_cast<interval_data*>(node->data)->high);
  }
  return 0;
} /* th_verify_nodes_int() */

int verify_nodes_rb(const struct bstree* const tree,
                    struct bstree_node* const  node) {
  const uint8_t* node_key = node->key;
  uint8_t*       left_key;
  uint8_t*       right_key;

  /*
   * Verify root and nil nodes are black (RBTree property #1)
   */
  struct bstree_node* nil = tree->nil;
  int                 height;
  CATCH_REQUIRE(tree->root->red == 0);
  CATCH_REQUIRE(tree->nil->red == 0);
  struct bstree_node* parent = tree->root->parent;
  CATCH_REQUIRE(parent == nil);

  /*
   * Verify children are < current node if a left child, and > current node
   * if a right child
   */
  if (node->left != nil && node->right != nil) {
    left_key  = node->left->key;
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
                     struct bstree_node* const  node) {
  const uint8_t*      node_key = node->key;
  uint8_t*            left_key;
  uint8_t*            right_key;
  struct bstree_node* nil = tree->nil;

  /*
   * Verify children are < current node if a left child, and > current node
   * if a right child (BSTree property #1)
   */
  if (node->left != nil && node->right != nil) {
    left_key  = node->left->key;
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
  CATCH_REQUIRE(bstree_node_height(tree, node) <= 10 * bstree_size(tree));

  return 0;
} /* th_verify_nodes_bst() */

int collect_low(const struct bstree* const, struct bstree_node* const node) {
  int32_t low;
  memcpy(&low, node->key, sizeof(low));
  g_inorder.push_back(low);
  return 0;
}

/* Recompute max_high bottom-up and compare with the stored value */
bool max_high_ok(const struct bstree*       t,
                 const struct inttree_node* n,
                 int32_t*                   subtree_max) {
  if (n == reinterpret_cast<const struct inttree_node*>(t->nil)) {
    *subtree_max = INT32_MIN;
    return true;
  }
  int32_t lmax, rmax;
  if (!max_high_ok(t, n->left, &lmax) || !max_high_ok(t, n->right, &rmax)) {
    return false;
  }
  auto* d      = reinterpret_cast<const struct interval_data*>(n->data);
  *subtree_max = std::max({lmax, rmax, d->high});
  return n->max_high == *subtree_max;
}

/* Recompute subtree counts and compare with the stored value */
bool count_ok(const struct bstree*      t,
              const struct ostree_node* n,
              int32_t*                  count) {
  if (n == reinterpret_cast<const struct ostree_node*>(t->nil)) {
    *count = 0;
    return true;
  }
  int32_t l, r;
  if (!count_ok(t, n->left, &l) || !count_ok(t, n->right, &r)) {
    return false;
  }
  *count = l + r + 1;
  return n->count == *count;
}

struct bstree* make_inttree(struct bstree_config* c) {
  memset(c, 0, sizeof(*c));
  c->elt_size = sizeof(struct interval_data);
  c->max_elts = -1;
  c->flags    = RCSW_DS_BSTREE_RB | RCSW_DS_BSTREE_INT;
  return inttree_init(nullptr, c);
}

} /* namespace th::bst */
