/**
 * \file ostree_node.h
 * \cond INTERNAL
 * \ingroup ds
 * \brief Functions related to ostree nodes
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/ostree.h"

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
/**
 * Whether the last operation was an insert or delete (fixup procedure is
 * different)
 */
enum ostree_fixup_type {
    ekOSTREE_FIXUP_INSERT,
    ekOSTREE_FIXUP_DELETE,
};

/*******************************************************************************
 * RCSW Private Functions
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Fixup count field for all nodes above target node after an
 * insertion/deletion.
 *
 * \param tree The ostree handle.
 * \param node The node to fixup.
 * \param type The type of fixup to perform.
 */
RCSW_LOCAL void ostree_count_fixup(const struct bstree* tree,
                        struct ostree_node * node,
                        enum ostree_fixup_type type);
/**
 * \brief Update the subtree size for a node.
 *
 * \param node The node to update.
 */
RCSW_LOCAL void ostree_node_update_count(struct ostree_node* node);

END_C_DECLS

/* \endcond */
