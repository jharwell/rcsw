/**
 * \file
 *
 * \copyright 2023 John Harwell
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/bstree.h"
#include "rcsw/ds/inttree.h"
#include "rcsw/ds/ostree.h"
#include "rcsw/ds/rbtree.h"
#include <vector>

/*******************************************************************************
 * Namespaces/Decls
 ******************************************************************************/
namespace th::bst {

/*******************************************************************************
 * Type Definitions
 ******************************************************************************/
using bst_verify_cb = int (*)(const struct bstree *const tree,
                              struct bstree_node *const node);

using test_t = void (*)(int len, struct bstree_config *config,
                        bst_verify_cb verify_cb);
using rm_test_t = void (*)(int len, int remove_type,
                           struct bstree_config *config,
                           bst_verify_cb verify_cb);

/*******************************************************************************
 * Forward Decls
 ******************************************************************************/
/**
 * \brief Verify parent-child relationships in a BSTREE (RB)
 *
 * \return 0 if OK, nonzero otherwise
 */
int verify_nodes_rb(const struct bstree *const tree,
                    struct bstree_node *const node);

/**
 * \brief Verify parent-child relationships in a BSTREE
 *
 * \return 0 if OK, nonzero otherwise
 */
int verify_nodes_bst(const struct bstree *const tree,
                     struct bstree_node *const node);

/**
 * \brief Verify parent-child relationships in an interval tree.
 *
 * \return 0 if OK, nonzero otherwise
 */
int verify_nodes_int(const struct bstree *const tree,
                     struct bstree_node *const node);

extern std::vector<int32_t> g_inorder;

int collect_low(const struct bstree *const, struct bstree_node *const node);

/* Recompute max_high bottom-up and compare with the stored value */
bool max_high_ok(const struct bstree *t, const struct inttree_node *n,
                 int32_t *subtree_max);

/* Recompute subtree counts and compare with the stored value */
bool count_ok(const struct bstree *t, const struct ostree_node *n,
              int32_t *count);
struct bstree *make_inttree(struct bstree_config *c);

/*
 * Caller-owned storage for one tree. Under RCSW_CONFIG_NOALLOC the library
 * cannot allocate, so with_storage() gives \p c element and node pools (and
 * bounds an unbounded max_elts to kTreeElts) and returns the handle to pass to
 * X_init(). In heap builds it leaves \p c alone and returns NULL, so the
 * library allocates.
 */
enum class tree_kind { bstree, ostree, inttree };
constexpr int kTreeElts = 256;
struct tree_storage {
  struct bstree handle;
  std::vector<dptr_t> elements;
  std::vector<dptr_t> nodes;
};
struct bstree *with_storage(tree_storage &s,
                            struct bstree_config *c,
                            tree_kind kind = tree_kind::bstree);
} /* namespace th::bst */
