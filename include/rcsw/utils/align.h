/**
 * \file
 *
 * \copyright 2017 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * \ingroup utils
 *
 * \brief Alignment related macros.
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdint.h>

#include "rcsw/core/compilers.h" /* typeof */

/*******************************************************************************
 * Macros
 ******************************************************************************/
/** \brief True if pointer \p p is aligned to \p byte_count bytes. */
#define RCSW_IS_MEM_ALIGNED(p, byte_count) \
  ((((uintptr_t)(p)) % (byte_count)) == 0)

/**
 * \brief True if \p size is a multiple of \p power_of_two.
 *
 * \note If \p power_of_two is not a power of two you will get wrong results.
 */
#define RCSW_IS_SIZE_ALIGNED(size, power_of_two) \
  (((size) & ((power_of_two) - 1)) == 0)

/**
 * \brief Round \p size up to the next multiple of \p power_of_two.
 *
 * The result has the type of \p size. The mask is built in that type, so a
 * narrower \p power_of_two (e.g. \c 8U with a 64-bit \p size) cannot clear
 * the upper bits.
 *
 * \note If \p power_of_two is not a power of two you will get wrong results.
 */
#define RCSW_ALIGN_SIZE(size, power_of_two)        \
  (((size) + (typeof(size))((power_of_two) - 1)) & \
   ~(typeof(size))((power_of_two) - 1))
