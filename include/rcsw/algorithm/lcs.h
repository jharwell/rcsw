/**
 * \file lcs.h
 * \ingroup algorithm
 * \brief Longest Common Subsequence (LCS) of two sequences of characters.
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/common/common.h"

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/
/**
 * \brief Representation of algorithm to find the longest common subsequence of
 * two strings.
 */
struct lcs_calculator {
    size_t size;     /// Longest common subsequence length.
    size_t len_x;    /// Length of string #1.
    size_t len_y;    /// Length of string #2.
    const char * y;   /// String #1.
    const char * x;   /// String #2.
    int *results;    /// N x N array storing the optimal results route.
    char *sequence;  /// Longest common subsequence.
};

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Initialize a longest common subsequence (LCS) calculator
 *
 * \param lcs The LCS calculator to initialize
 * \param x Sequence # 1
 * \param y Sequence # 2
 *
 * \return \ref status_t
 */
status_t lcs_init(struct lcs_calculator * lcs,
                  const char * x,
                  const char * y);

/**
 * \brief Destroy a created LCS calculator
 *
 * \param lcs The LCS handle
 *
 * Any further use of the LCS after calling this function is undefined.
 *
 */
void lcs_destroy(struct lcs_calculator * lcs);

/**
 * \brief Compute the LCS of x,y using top down dynamic programming
 *
 * \param lcs The LCS handle
 *
 * \return LCS of (x,y) or -1 if an error occurred
 */
int lcs_rec(const struct lcs_calculator* lcs);

/**
 * \brief Compute LCS(x,y) using bottom up dynamic programming
 *
 * \param lcs The LCS handle
 *
 * \return \ref status_t
 */
int lcs_iter(struct lcs_calculator * lcs);

END_C_DECLS

