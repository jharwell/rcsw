/**
 * \file
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * \ingroup utils
 *
 * \brief Miscellaneous utility functions for numeric operations.
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/al/types.h"
#include "rcsw/core/compilers.h"

/*******************************************************************************
 * Public API
 ******************************************************************************/
BEGIN_C_DECLS
/**
 * \brief Clamp a float to [0, 255].
 *
 * Values may drift slightly outside this range after floating-point
 * scaling; this function corrects them.
 *
 * \param v The value to clamp.
 *
 * \return The clamped value.
 */
RCSW_CONST static inline float utils_clamp_f255(float v) {
  if (v < 0) {
    return 0.0F;
  }
  if (v > 255.0F) {
    return 255.0F;
  }
  return v;
}

/**
 * \brief Generate all permutations of elements [start, size) in \p arr.
 *
 * \p fp is called once with each permutation.
 *
 * \param arr      Array of elements to permute.
 * \param n_elts   Total number of elements in \p arr.
 * \param elt_size Size of each element in bytes.
 * \param start    Index from which to begin permuting (pass 0 for all).
 * \param fp       Callback invoked for each permutation. Receives a pointer
 *                 to the first element of the (reordered) array.
 */
RCSW_API void utils_permute(
  void* arr, size_t n_elts, size_t elt_size, size_t start, void (*fp)(void* arr));

/**
 * \brief Test whether all bytes of an element are zero.
 *
 * Uses pointer comparisons for elements up to \c sizeof(double); falls
 * back to a byte loop for larger elements.
 *
 * \param elt      Pointer to the element to test.
 * \param elt_size Size of the element in bytes.
 *
 * \return \ref bool_t TRUE if all bytes are zero.
 */
RCSW_API bool_t utils_zchk(void* elt, size_t elt_size);

END_C_DECLS
