/**
 * \file inttree.h
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
 * Macros
 ******************************************************************************/
/**
 * Convenience macro for getting a reference to the root node in the tree
 */
#define INTTREE_ROOT(tree) BSTREE_ROOT(tree)

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/
/**
 * A simple representation of an interval for use in a \ref inttree.
 */
struct interval_data {
    int32_t high;
    int32_t low;
};

/**
 * \brief A node in an \ref inttree.
 *
 * Note that the first fields are identical to the ones in the bstree_node; this
 * is necessary for the casting to "up" the inheritance tree to work.
 */
struct inttree_node {
    uint8_t key[RCSW_BSTREE_NODE_KEYSIZE];
    uint8_t *data;
    struct inttree_node *left;
    struct inttree_node *right;
    struct inttree_node *parent;
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
 * \brief Determine if an \ref inttree is full.
 *
 * \param tree The interval tree handle.
 *
 * \return \ref bool_t
 */
static inline bool_t inttree_isfull(const struct bstree* const tree) {
    return bstree_isfull(tree);
}

/**
 * \brief Determine if an \ref inttree is empty.
 *
 * \param tree The interval tree handle.
 *
 * \return \ref bool_t
 */
static inline bool_t inttree_isempty(const struct bstree* const tree) {
    return bstree_isempty(tree);
}

/**
 * \brief Get # elements currently in an \ref inttree.
 *
 * \param tree The interval tree handle.
 *
 * \return The # of elements, or 0 on error.
 */
static inline size_t inttree_n_elts(const struct bstree* const tree) {
    return bstree_n_elts(tree);
}

/**
 * \brief Calculate the # of bytes that the \ref inttree will require if \ref
 * RCSW_DS_NOALLOC_DATA is passed to manage a specified # of elements of a
 * specified size.
 *
 * \param max_elts # of desired elements the tree will hold
 *
 * \return The total # of bytes the application would need to allocate
 */
static inline size_t inttree_element_space(size_t max_elts) {
    return bstree_element_space(max_elts, sizeof(struct interval_data));
}

/**
 * \brief Calculate the space needed for the nodes in the \ref inttree, given a
 * max # of elements
 *
 * Used in conjunction with \ref RCSW_DS_NOALLOC_NODES.
 *
 * \param max_elts # of desired elements the tree will hold
 *
 * \return The # of bytes required
 */
static inline size_t inttree_node_space(size_t max_elts) {
    return bstree_node_space(max_elts);
}

static inline status_t inttree_insert(struct bstree* tree,
                                      struct interval_data* interval) {
  return  bstree_insert_internal(tree,
                                 &(interval)->low,
                                 interval,
                                 sizeof(struct inttree_node));
}

static inline struct bstree* inttree_init(struct bstree* const tree_in,
                                          const struct ds_params* const params) {
  return bstree_init_internal(tree_in, params, sizeof(struct inttree_node));
}

static inline status_t inttree_delete(struct bstree* tree,
                                     struct inttree_node* victim,
                                     void* elt) {
  return bstree_delete(tree, (struct bstree_node*)victim, elt);
}

static inline status_t inttree_remove(struct bstree* tree, const void* key) {
  return bstree_remove(tree, key);
}

/*******************************************************************************
 * API Functions
 *
 * There are other bstree functions that you can use besides these; however,
 * given that you are using an Interval tree, these are really the only
 * operations you should be doing (besides insert/delete). I don't wrap the
 * bstree API here for that reason.
 *
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Determine if the given interval overlaps any in the \ref inttree.
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
struct inttree_node* inttree_overlap_search(
    const struct bstree * tree,
    struct inttree_node * root,
    const struct interval_data * interval);

/**
 * \brief Initialize \ref inttree specific bits of a BST
 *
 * Do not call this function directly.
 *
 * \param tree The partially constructed interval tree.
 */
void inttree_init_helper(const struct bstree * tree);

/**
 * \brief Fixup high field for all nodes above target node after an
 * insertion/deletion
 *
 * Do not call this function directly.
 *
 * \param tree The interval tree handle.
 * \param node The leaf node to propagate fixes up the tree for.
 */
void inttree_high_fixup(const struct bstree* tree,
                         struct inttree_node * node);

/**
 * \brief Compare two the keys of two intervals during insertion
 *
 * \param a Key #1
 * \param b Key #2
 *
 * \return <,=,> 0, depending if low endpoint of a <,=,> low endpoint of b
 */
int inttree_cmp_key(const void * a, const void * b) RCSW_PURE;

END_C_DECLS
