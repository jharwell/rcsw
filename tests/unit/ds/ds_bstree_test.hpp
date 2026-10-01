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
typedef int (*int_verify_cb)(const struct bstree *const tree,
                             struct inttree_node *const node);

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
                     struct inttree_node *const node);

extern std::vector<int32_t> g_inorder;

int collect_low(const struct bstree *const, struct bstree_node *const node);

/* Recompute max_high bottom-up and compare with the stored value */
bool max_high_ok(const struct bstree *t, const struct inttree_node *n,
                 int32_t *subtree_max);

/* Recompute subtree counts and compare with the stored value */
bool count_ok(const struct bstree *t, const struct ostree_node *n,
              int32_t *count);
struct bstree *make_inttree(struct bstree_config *c);
} /* namespace th::bst */
