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
#include "rcsw/ds/inttree.h"

/*******************************************************************************
 * Private API
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
