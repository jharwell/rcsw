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
 * This file is part of RCSW.
 *
 * RCSW is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * RCSW is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * RCSW.  If not, see <http://www.gnu.org/licenses/
 */

#ifndef INCLUDE_RCSW_DS_ADJ_LIST_H_
#define INCLUDE_RCSW_DS_ADJ_LIST_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/common/common.h"

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

#endif /* INCLUDE_RCSW_DS_ADJ_LIST_H_ */
