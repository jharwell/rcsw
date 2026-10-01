/**
 * \file
 *
 * \copyright 2017 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * \ingroup algorithm
 *
 * \brief Useful algorithms for arrays, lists, matrices, sequences, etc.
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stddef.h>

#include "rcsw/al/types.h"
#include "rcsw/core/compilers.h"

/*******************************************************************************
 * Public API
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
 * \return true if such a parenthesization exists; false if not, or if a
 * parameter is invalid.
 */
RCSW_API bool_t str_is_parenthesizable(const char* x,
                                       char*       r,
                                       char        el,
                                       char (*multiply_cb)(char x, char y));

END_C_DECLS
