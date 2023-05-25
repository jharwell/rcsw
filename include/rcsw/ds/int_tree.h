/**
 * \file int_tree.h
 * \ingroup ds
 * \brief Implementation of interval tree data structure.
 *
 * Built on top of the \ref bstree module.
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
 * A simple representation of an interval for use in an interval tree.
 */
struct interval_data {
    int32_t high;
    int32_t low;
};

/**
 * \brief A node in an interval tree.
 *
 * Note that the first fields are identical to the ones in the bstree_node; this
 * is necessary for the casting to "up" the inheritance tree to work.
 */
struct int_tree_node {
    uint8_t key[BSTREE_NODE_KEYSIZE];
    uint8_t *data;
    struct int_tree_node *left;
    struct int_tree_node *right;
    struct int_tree_node *parent;

    /**
     * If the tree is used as a red-black tree, then this field is used to
     * indicate the color of the node. It is undefined for regular BSTs. If
     * red=FALSE then the node is black.
     */
    bool_t red;

    /**
     * This field is the largest HIGH value of the subtree rooted at this node.
     */
    int32_t max_high;
};

/*******************************************************************************
 * Inline Functions
 ******************************************************************************/
/**
 * \brief Determine if an interval tree is full.
 *
 * \param tree The interval tree handle.
 *
 * \return \ref bool_t
 */
static inline bool_t int_tree_isfull(const struct bstree* const tree) {
    return bstree_isfull(tree);
}

/**
 * \brief Determine if an interval tree is empty.
 *
 * \param tree The interval tree handle.
 *
 * \return \ref bool_t
 */
static inline bool_t int_tree_isempty(const struct bstree* const tree) {
    return bstree_isempty(tree);
}

/**
 * \brief Get # elements currently in an interval tree.
 *
 * \param tree The interval tree handle.
 *
 * \return The # of elements, or 0 on error.
 */
static inline size_t int_tree_n_elts(const struct bstree* const tree) {
    return bstree_n_elts(tree);
}

/**
 * \brief Calculate the # of bytes that the interval tree will require if
 * \ref DS_APP_DOMAIN_DATA is passed to manage a specified # of elements of a
 * specified size.
 *
 * \param max_elts # of desired elements the tree will hold
 *
 * \return The total # of bytes the application would need to allocate
 */
static inline size_t int_tree_element_space(size_t max_elts) {
    return bstree_element_space(max_elts, sizeof(struct interval_data));
}

/**
 * \brief Calculate the space needed for the nodes in the interval tree, given a
 * max # of elements
 *
 * Used in conjunction with \ref DS_APP_DOMAIN_NODES.
 *
 * \param max_elts # of desired elements the tree will hold
 *
 * \return The # of bytes required
 */
static inline size_t int_tree_node_space(size_t max_elts) {
    return bstree_node_space(max_elts);
}


/*******************************************************************************
 * Macros
 ******************************************************************************/
/** Convenience macro for getting a reference to the root node in the tree */
#define INT_TREE_ROOT(tree) BSTREE_ROOT(tree)

/**
 * \brief Insert an item into an interval tree
 *
 * \param tree The interval tree handle
 * \param interval The interval to insert
 */
#define int_tree_insert(tree, interval)                                 \
    bstree_insert_internal(tree, &(interval)->low,                      \
                           interval, sizeof(struct int_tree_node))
/**
 * \brief Initialize an interval tree
 *
 * \param tree_in The interval tree handle
 * \param params Initialization parameters
 */
#define int_tree_init(tree_in, params)                                  \
    bstree_init_internal(tree_in, params, sizeof(struct int_tree_node))

#define int_tree_delete bstree_delete
#define int_tree_remove bstree_remove

/*******************************************************************************
 * Function Prototypes
 *
 * There are other bstree functions that you can use besides these; however,
 * given that you are using an Interval tree, these are really the only
 * operations you should be doing (besides insert/delete). I don't wrap the
 * bstree API here for that reason.
 *
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Determine if the given interval overlaps any in the tree.
 *
 * The initial call to this function should (probably) pass tree->root as the
 * root node, in order to search the entire tree.
 *
 * \param tree The interval tree handle
 * \param root Root of tree to start searching at
 * \param interval The interval to test for overlaps with.
 *
 * \return The first overlapping interval encountered, or NULL if none was found
 * or an error occurred.
 */
struct int_tree_node* int_tree_overlap_search(
    const struct bstree * tree,
    struct int_tree_node * root,
    const struct interval_data * interval);

/**
 * \brief Initialize Interval tree-specific bits of a BST
 *
 * Do not call this function directly.
 *
 * \param tree The partially constructed interval tree.
 */
void int_tree_init_helper(const struct bstree * tree);

/**
 * \brief Fixup high field for all nodes above target node after an
 * insertion/deletion
 *
 * Do not call this function directly.
 *
 * \param tree The interval tree handle.
 * \param node The leaf node to propagate fixes up the tree for.
 */
void int_tree_high_fixup(const struct bstree* tree,
                         struct int_tree_node * node);

/**
 * \brief Compare two the keys of two intervals during insertion
 *
 * \param a Key #1
 * \param b Key #2
 *
 * \return <,=,> 0, depending if low endpoint of a <,=,> low endpoint of b
 */
int int_tree_cmp_key(const void * a, const void * b) RCSW_PURE;

END_C_DECLS

