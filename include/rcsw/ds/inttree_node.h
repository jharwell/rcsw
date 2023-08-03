/**
 * \file inttree_node.h
 * \cond INTERNAL
 * \ingroup ds
 * \brief Functions related to interval tree nodes.
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/inttree.h"

/*******************************************************************************
 * RCSW Private Functions
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Update the max high interval for a node during the fixup process after
 * an insertion/deletion.
 *
 * \param node The node to update.
 */
void inttree_node_update_max(struct inttree_node* node);

/* \endcond */

END_C_DECLS
