/**
 * \file edit_dist.h
 * \brief Algorithm for finding the edit distance between two
 * sequences of objects.
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
 * Structure Definitions
 ******************************************************************************/
/**
 * \brief Algorithm handle for finding the edit distance between two contiguous
 * sequences of objects.
 */
struct edit_dist_finder {
  /**
   * Sequence #1.
   */
  const void *seq_a;

  /**
   * Sequence #1.
   */
  const void *seq_b;

  /**
   * Size of each element in a sequence in bytes.
   */
  size_t elt_size;

  /**
   * Memoization table used with a recursive implementation.
   */
  int* memoization;

  /**
   * Compare two elements for equality.
   */
  bool_t (*cmpe)(const void* const e1,
                 const void* const e2);

  /**
   * Get the length of a sequence.
   */
  size_t (*seq_len)(const void* const seq);
};

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Initialize an edit distance finder
 *
 * \param finder The finder to initialize
 *
 * \param a Pointer to sequence #1
 *
 * \param b Pointer to sequence #2
 *
 * \param elt_size Size of elements in bytes
 *
 * \param cmpe Callback for comparing if two sequences are equivalent or not
 *
 * \param seq_len Callback for obtaining the length of a sequence
 *
 * \return \ref status_t
 */
RCSW_API status_t edit_dist_init(struct edit_dist_finder * finder,
                                 const void* a,
                        const void*b,
                        size_t elt_size,
                        bool_t (*cmpe)(const void* e1,
                                       const void* e2),
                        size_t (*seq_len)(const void* seq));
/**
 * \brief Destroy an edit distance finder
 *
 * \param finder The finder handle
 *
 */
RCSW_API void edit_dist_destroy(struct edit_dist_finder * finder);

/**
 * \brief Find the edit distance between two sequences
 *
 * \param finder The edit distance finder handle
 *
 * \param type The type of method to use: iterative or recursive with
 *             memoization
 *
 * \return The edit distance, or -1 on error
 */
RCSW_API int edit_dist_find(struct edit_dist_finder * finder,
                            enum exec_type type);
END_C_DECLS
