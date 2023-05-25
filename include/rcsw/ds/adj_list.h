/**
 * \file adj_list.h
 * \ingroup ds
 * \brief Implementation of adj list representation for graphs.
 *
 * Pros: O(|V| + |E|) space. Adding a vertex is more efficient than with an \ref
 * adj_matrix representation
 *
 * Cons: Queries like "is there an edge from vertex u to vertex v" are not
 * efficient and are O(|V|).
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/rcsw.h"

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/

/*******************************************************************************
 * Macros
 ******************************************************************************/

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
BEGIN_C_DECLS

status_t adj_list_init(const struct ds_params* const params);
status_t adj_list_destroy(struct adj_list* const list);
status_t adj_list_edge_add(struct adj_list* const list,
                             size_t u, size_t v, const void* const w);
status_t adj_list_edge_remove(struct adj_list* const list,
                                size_t u, size_t v);
bool_t adj_list_edge_query(struct adj_list* const list,
                             size_t u, size_t v);

END_C_DECLS

