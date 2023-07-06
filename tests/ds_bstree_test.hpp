/**
 * \file ds_bstree_test.hpp
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/bstree.h"
#include "rcsw/ds/ostree.h"
#include "rcsw/ds/rbtree.h"

/*******************************************************************************
 * Type Definitions
 ******************************************************************************/
using bst_verify_cb = int(*)(const struct bstree* const tree,
                             struct bstree_node * const node);

using ds_bstree_test_t = void(*)(int len,
                                 struct bstree_params *params,
                                 bst_verify_cb verify_cb);
using ds_bstree_rm_test_t = void (*)(int len,
                                     int remove_type,
                                     struct bstree_params *params,
                                     bst_verify_cb verify_cb);
typedef int (*int_verify_cb)(const struct bstree* const tree,
                             struct inttree_node * const node);

/*******************************************************************************
 * Forward Decls
 ******************************************************************************/
/**
 * \brief Verify parent-child relationships in a BSTREE (RB)
 *
 * \return 0 if OK, nonzero otherwise
 */
int th_verify_nodes_rb(const struct bstree* const tree,
                       struct bstree_node * const node);

/**
 * \brief Verify parent-child relationships in a BSTREE
 *
 * \return 0 if OK, nonzero otherwise
 */
int th_verify_nodes_bst(const struct bstree* const tree,
                        struct bstree_node * const node);

/**
 * \brief Verify parent-child relationships in an interval tree.
 *
 * \return 0 if OK, nonzero otherwise
 */
int th_verify_nodes_int(const struct bstree* const tree,
                        struct inttree_node * const node);
