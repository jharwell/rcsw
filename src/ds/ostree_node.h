/**
 * \file
 *
 * \copyright 2017 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * \cond INTERNAL
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/core/compilers.h"
#include "rcsw/ds/bstree.h"
#include "rcsw/ds/ostree.h"

/*******************************************************************************
 * Private API
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Update the subtree size for a node.
 *
 * \param node The node to update.
 */
RCSW_LOCAL void ostree_node_update_count(struct ostree_node* node);

END_C_DECLS

/* \endcond */
