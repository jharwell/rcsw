/**
 * \file bstree_node.h
 * \cond INTERNAL
 * \ingroup ds
 * \brief Functions for BST nodes.
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef INCLUDE_RCSW_DS_BSTREE_NODE_H_
#define INCLUDE_RCSW_DS_BSTREE_NODE_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/bstree.h"

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Create a node in a BST variant
 *
 * Allocates node, data according to configuration parameters of BST variant.
 *
 * \param tree The BST handle
 * \param parent The parent of the node to be created. Can be NULL.
 * \param key_in The key for the new node. Can be NULL.
 * \param data_in The data for the new node. Can be NULL.
 * \param node_size Size of the node in bytes.
 *
 * \return The created node, or NULL if an error occurred
 */
struct bstree_node *bstree_node_create(const struct bstree * tree,
                                       struct bstree_node * parent,
                                       void * key_in, void * data_in,
                                       size_t node_size);

/**
 * \brief Delete part or all of a BST node
 *
 * This function deletes the BSTREE node and/or the data it keeps track of,
 * depending on configuration flags.
 *
 * \param tree The BST handle
 * \param node The node to delete
 *
 * \return Always returns 0. This is necessary so that this function can be used
 * as a callback during node traversal
 */
int bstree_node_destroy(const struct bstree * tree,
                         struct bstree_node *node);

/**
 * \brief Allocate a node in a bstree
 *
 * Not necessarily a bstree_node--could be an RB node, for example, hence
 * the size parameter. Note that although the return type is a
 * bstree_node*, the actual amount of space allocated could be more.
 *
 * \param tree The BST handle
 * \param node_size Size of the node to create in bytes
 *
 * \return The allocated bstree_node, or NULL if no suitable node was found
 */
struct bstree_node *bstree_node_alloc(const struct bstree * tree,
                                      size_t node_size);

/**
 * \brief Deallocate a node in a BST
 *
 * \param tree The BST handle
 * \param node The node to deallocate
 */
void bstree_node_dealloc(const struct bstree * tree,
                         struct bstree_node *node);

/**
 * \brief Allocate a datablock for a BST node
 *
 * \param tree The BST handle
 *
 * \return The allocated datablock, or NULL if no valid block could be found
 */
void *bstree_node_datablock_alloc(const struct bstree * tree);

/**
 * bstree_node_datablock_dealloc() - Deallocate a datablock
 *
 * RETURN:
 *     N/A
 *
 **/
void bstree_node_datablock_dealloc(const struct bstree * tree,
                                   uint8_t *datablock);

/**
 * \brief Internal function to do pre-order traversal
 *
 * \param tree The BST handle
 * \param node The start of the traversal (does not have to be the root of the tree)
 * \param cb Callback called on each node in the tree, and passed the BST
 * handle. Cannot be used to modify the BST handle, but CAN modify the node.
 *
 * \return Return code of last callback that was non-zero, or 0 if callback
 * succeeded on all nodes
 */
int bstree_traverse_nodes_preorder(struct bstree * tree,
                                   struct bstree_node * node,
                                   int (*cb)(const struct bstree * tree,
                                             struct bstree_node * node));

/**
 * \brief Internal function to do in-order traversal
 *
 * \param tree The BST handle
 * \param node The start of the traversal (does not have to be the root of the
 * tree)
 * \param cb Callback called on each node in the tree, and passed the BST
 * handle. Cannot be used to modify the BST handle, but CAN modify the node.
 *
 * \return Return code of last callback that was non-zero, or 0 if callback
 * succeeded on all nodes
 */
int bstree_traverse_nodes_inorder(struct bstree * tree,
                                  struct bstree_node * node,
                                  int (*cb)(const struct bstree * tree,
                                            struct bstree_node * node));

/**
 * \brief Internal function used to perform a post-order traversal (this is
 * needed when doing things like deleting nodes/data from the tree--in order
 * traversal will not work).
 *
 * \param tree The BST handle
 * \param node The start of the traversal (does not have to be the root of the
 * tree)
 * \param cb Callback called on each node in the tree, and passed the BST
 * handle. Cannot be used to modify the BST handle, but CAN modify the node.
 *
 * \return Return code of last callback that was non-zero, or 0 if callback
 * succeeded on all nodes
 */
int bstree_traverse_nodes_postorder(struct bstree * tree,
                                    struct bstree_node * node,
                                    int (*cb)(const struct bstree * tree,
                                              struct bstree_node * node));

/**
 * \brief - Rotate the subtree anchored at a BST node to the left
 *
 * Rotate to the left about "node". This function rotate's node's parent
 * to become node's right child. Node's right child will become the former
 * parent's left child.
 *
 * \param tree The BST handle
 * \param node The node in the tree to rotate about
 *
 */
void bstree_node_rotate_left(struct bstree * tree,
                             struct bstree_node *node);

/**
 * \brief Rotate the subtree anchored at a bstree node to the left
 *
 * Note that no special case for the root is necessary because of the use of
 * tree->nil sentinel
 *
 * \param tree The BST handle
 * \param node The node in the tree to rotate about
 */
void bstree_node_rotate_right(struct bstree * tree,
                              struct bstree_node *node);

/**
 * \brief Print a BSTREE node
 *
 * \param tree The BST handle
 * \param node The node to print
 *
 * \return Always returns 0. This is necessary so that it can be used as a
 * callback to print a bstree
 *
 */
int bstree_node_print(const struct bstree * tree,
                      const struct bstree_node * node);

/**
 * \brief Get the height of a binary search tree anchored at a node
 *
 * \param tree The BST handle.
 * \param node The root of the subtree to get the height of.
 *
 * \return The height or 0 if the node is NULL
 */
size_t bstree_node_height(const struct bstree* tree,
                          const struct bstree_node * node) RCSW_PURE;

/**
 * \brief Get the successor of x (smallest node larger than x)
 *
 * Uses the algorithm in _Introduction_To_Algorithms_
 *
 * \param tree The BST handle
 * \param node The node to print
 *
 * \return The successor, or NULL if none was found (i.e. x is d the largest
 * node in the tree)
 */
struct bstree_node* bstree_node_successor(const struct bstree* tree,
                                          const struct bstree_node* node) RCSW_PURE;
END_C_DECLS

/* \endcond */

#endif /* INCLUDE_RCSW_DS_BSTREE_NODE_H_ */
