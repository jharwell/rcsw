/**
 * \file
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * \cond INTERNAL
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
