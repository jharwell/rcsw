/**
 * \file edit_dist.h
 * \brief Algorithm for finding the edit distance between two
 * sequences of objects.
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
#ifndef INCLUDE_RCSW_ALGORITHM_EDIT_DIST_H_
#define INCLUDE_RCSW_ALGORITHM_EDIT_DIST_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/common/common.h"

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
/**
 * \brief The runtime implementation method to use for the edit distance
 * calculation.
 */
enum edit_dist_exec_type {
  EDIT_DIST_REC,   /// Use a recursive runtime implementation
  EDIT_DIST_ITER,  /// Use a iterative runtime implementation
};

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/
/**
 * \brief Representation of the algorithm for finding the edit distance between
 * two contiguous sequences of objects.
 */
struct edit_dist_finder {
    const void *a_;
    const void *b_;
    size_t el_size_;
    size_t n_elts_;
    int* c_;
    bool_t (*cmpe_)(const void* const e1,
                    const void* const e2);
    size_t (*seq_len_)(const void* const seq);
    size_t edit_dist_;
};

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Initialize an edit distance finder
 *
 * \param finder The finder to initialize
 * \param a Pointer to sequence #1
 * \param b Pointer to sequence #2
 * \param el_size Size of elements in bytes
 * \param cmpe Callback for comparing if two sequences are equivalent or not
 * \param seq_len Callback for obtaining the length of a sequence
 *
 * \return \ref status_t
 */
status_t edit_dist_init(struct edit_dist_finder * finder,
                        const void* a,
                        const void*b,
                        size_t el_size,
                        bool_t (*cmpe)(const void* e1,
                                        const void* e2),
                        size_t (*seq_len)(const void* seq));
/**
 * \brief Destroy an edit distance finder
 *
 * \param finder The edit distance finder handle
 *
 */
void edit_dist_destroy(struct edit_dist_finder * finder);

/**
 * \brief Find the edit distance between two sequences
 *
 * \param finder The edit distance finder handle
 * \param type The type of method to use: iterative or recursive with memoization
 *
 * \return The edit distance, or -1 on error
 */
int edit_dist_find(struct edit_dist_finder * finder,
                   enum edit_dist_exec_type type);
END_C_DECLS

#endif /* INCLUDE_RCSW_ALGORITHM_EDIT_DIST_H_ */
