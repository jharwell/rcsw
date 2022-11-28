/**
 * \file algorithm.h
 * \ingroup algorithm
 * \brief Useful algorithms for arrays, lists, matrices, sequences, etc.
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef INCLUDE_RCSW_ALGORITHM_ALGORITHM_H_
#define INCLUDE_RCSW_ALGORITHM_ALGORITHM_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/common/common.h"

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Deterimine if a parenthesization for x exists such that the value of
 * the resulting expression is el, where el is some element of the alphabet.
 *
 * \param x String to attempt to parenthesize
 * \param r Result matrix (assumed to be |x| by |x|)
 * \param el The goal char
 * \param multiply_cb Callback to multiple two chars in the alphabet
 *
 * If an invalid parameter is passed, the algorithm will return FALSE.
 *
 * \return \ref bool_t
 **/
bool_t str_is_parenthesizable(const char *x,
                              char *r, char el,
                              char (*multiply_cb)(char x, char y));

/**
 * \brief Find the largest # in an array of non-negative integers
 *
 * \param array The array to search
 * \param n_elts # elements in array
 *
 * \return The largest #, or 0 if an ERROR occurred
 */
size_t alg_arr_largest_num(const size_t * array, size_t n_elts);

/**
 * \brief Find the largest # in an array of integers
 *
 * \param array The array to search
 * \param n_elts # elements in array
 *
 * \return The largest #, or 0 if an ERROR occurred
 */
int alg_arr_largest_num2(const int * array, size_t n_elts);

END_C_DECLS

#endif /*  INCLUDE_RCSW_ALGORITHM_ALGORITHM_H_  */
