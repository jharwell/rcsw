/**
 * \file ostree.h
 * \ingroup ds
 * \brief Order Statistics Tree implementation.
 *
 * Built on top of \ref bstree.
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/bstree.h"

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/
/**
 * \brief A node in an ostree, derived from a bstree_node (share common fields).
 */
struct ostree_node {
    uint8_t key[BSTREE_NODE_KEYSIZE];
    uint8_t *data;
    struct ostree_node *left;
    struct ostree_node *right;
    struct ostree_node *parent;
    bool_t red;

    int count;  /// Size of subtree anchored at node (including node)
};

/*******************************************************************************
 * Macros
 ******************************************************************************/
/* convienence macros for common operations */
#define OSTREE_ISFULL(tree) BSTREE_ISFULL(tree)
#define OSTREE_ISEMPTY(tree) BSTREE_ISEMPTY(tree)
#define OSTREE_N_ELTS(tree) BSTREE_N_ELTS(tree)
#define OSTREE_ROOT(tree) BSTREE_ROOT(tree)

/*
 * Use with DS_APP_DOMAIN_DATA/DS_APP_DOMAIN_NODES to calculate the size of the
 * block to store the elements the tree will manage/the bstree_nodes
 * should be. The +2 for the nodes is for the root and nil nodes, which are
 * always allocated. The root and nil nodes also always have a datablock
 * assigned to them.
 */
#define OSTREE_CALC_ELEMENT_SPACE(max_elts, el_size)    \
    bstree_element_space(max_elts, el_size)
#define OSTREE_CALC_NODE_SPACE(max_elts)                                \
  (ds_calc_meta_space((max_elts)+2) + sizeof(struct ostree_node) * ((max_elts)+2))

#define ostree_init(tree, params)                                       \
    bstree_init_internal(tree, params, sizeof(struct ostree_node))

#define ostree_insert(tree, key, data)                                  \
    bstree_insert_internal(tree, key, data, sizeof(struct ostree_node))
#define ostree_delete bstree_delete
#define ostree_remove bstree_remove

/*******************************************************************************
 * Function Prototypes
 *
 * There are other bstree functions that you can use besides these; however,
 * given that you are using an Order Statistics tree, these are really the only
 * operations you should be doing (besides insert/delete). I don't wrap the
 * bstree API here for that reason.
 *
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Select the ith smallest element in the OS-Tree.
 *
 * \param tree The ostree handle.
 * \param node The node to start searching at (should probably be the root).
 * \param i The element rank to select.
 *
 * \return The ith smallest element, or NULL if no such element or an error
 * occurred.
 */
struct ostree_node* ostree_select(const struct bstree* tree,
                                  struct bstree_node * node_in, size_t i);

/**
 * \brief Get the rank of an element within an OS-tree.
 *
 * \param tree The ostree handle.
 * \param node The node to get the rank of.
 *
 * \return The rank, or -1 on ERROR.
 */
int ostree_rank(const struct bstree * tree,
                const struct ostree_node* node);

/**
 * \brief Initialize OSTree specific bits of a BST.
 *
 * Do not call this function directly.
 * \param tree The ostree handle.
 */
void ostree_init_helper(struct bstree* tree);

END_C_DECLS

