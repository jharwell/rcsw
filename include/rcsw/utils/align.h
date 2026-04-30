/**
 * \file
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * \ingroup utils
 *
 * \brief Alignment related macros.
 */

#pragma once

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
 * \note If \p power_of_two is not a power of two you will get wrong results.
 */
#define RCSW_ALIGN_SIZE(size, power_of_two) \
  (((size) + (power_of_two) - 1) & ~((power_of_two) - 1))
