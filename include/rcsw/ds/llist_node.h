/**
 * \file llist_node.h
 * \cond INTERNAL
 * \ingroup ds
 * \brief Functions relating to linked list nodes.
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/llist.h"
#include "rcsw/common/fpc.h"

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Allocate a llist_node according to the memory domain for the nodes and
 * elements for the linked list.
 *
 * \param list The linked list handle.
 *
 * \return The allocated node, or NULL if no node could be found.
 */
struct llist_node *llist_node_alloc(struct llist * list);

/**
 * \brief Deallocate a llist_node.
 *
 * \param list The linked list handle.
 * \param node The node to deallocate from the linked list.
 */
void llist_node_dealloc(struct llist * list, struct llist_node *node);

/**
 * \brief Create a linked list node by allocating space for it and its data.
 *
 * The data is copied into the linked list node.
 *
 * \param list The linked list handle.
 * \param data_in The data for the new node.
 *
 * \return The created node, or NULL if an error occurred.
 */
struct llist_node * llist_node_create(struct llist * list,
                                      void * data_in);
/**
 * \brief Delete a linked list by deallocating its data, and then the node
 * itself.
 *
 * \param list The linked list handle.
 * \param node The node to destroy.
 */
void llist_node_destroy(struct llist * list, struct llist_node *node);

/**
 * \brief Allocate a datablock owned by a linked list.
 *
 * Depending on configuration parameters for the linked list, space for the
 * datablock may be malloc()ed.
 *
 * \param list The linked list handle.
 *
 * \return The allocated datablock, or NULL if no valid block could be found.
 */
dptr_t* llist_node_datablock_alloc(struct llist * list);

/**
 * \brief Deallocate a datablock belonging to a linked list.
 *
 * Depending on configuration parameters for the linked list, space for the
 * datablock may be free()ed.
 *
 * \param list The linked list handle.
 * \param datablock The datablock to dealloc.
 */
void llist_node_datablock_dealloc(struct llist * list, dptr_t* datablock);

END_C_DECLS

/* \endcond */
