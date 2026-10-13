/**
 * \file
 *
 * \copyright 2017 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * \ingroup utils
 *
 * \brief Alignment helpers.
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stddef.h>
#include <stdint.h>

#include "rcsw/al/types.h"
#include "rcsw/core/compilers.h"
#include "rcsw/core/fpc.h"

/*******************************************************************************
 * Public API
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief True if \p n is a power of two (0 is not).
 */
static inline bool_t utils_is_pow2(size_t n) {
  return n != 0 && (n & (n - 1)) == 0;
}

/**
 * \brief True if address \p addr is aligned to \p byte_count bytes.
 *
 * \p byte_count need not be a power of two, but must be nonzero (function
 * precondition; false is returned under \c LIBRA_FPC_RETURN). Address 0 is
 * aligned to everything.
 */
static inline bool_t utils_is_addr_aligned(uintptr_t addr, size_t byte_count) {
  RCSW_FPC_NV(false, byte_count > 0);
  return (addr % byte_count) == 0;
}

/**
 * \brief True if pointer \p p is aligned to \p byte_count bytes.
 *
 * As \ref utils_is_addr_aligned(), for a pointer. NULL is aligned to
 * everything.
 */
static inline bool_t utils_is_mem_aligned(const void* const p,
                                          size_t            byte_count) {
  return utils_is_addr_aligned((uintptr_t)p, byte_count);
}

/**
 * \brief True if \p size is a multiple of \p power_of_two.
 *
 * \p power_of_two must be a power of two (function precondition; false is
 * returned under \c LIBRA_FPC_RETURN).
 */
static inline bool_t utils_is_size_aligned(size_t size, size_t power_of_two) {
  RCSW_FPC_NV(false, utils_is_pow2(power_of_two));
  return (size & (power_of_two - 1)) == 0;
}

/**
 * \brief Round \p size up to the next multiple of \p power_of_two.
 *
 * \p power_of_two must be a power of two (function precondition; 0 is returned
 * under \c LIBRA_FPC_RETURN). The result wraps if it does not fit in a \c
 * size_t.
 */
static inline size_t utils_align_size(size_t size, size_t power_of_two) {
  RCSW_FPC_NV(0, utils_is_pow2(power_of_two));
  return (size + (power_of_two - 1)) & ~(power_of_two - 1);
}

END_C_DECLS
