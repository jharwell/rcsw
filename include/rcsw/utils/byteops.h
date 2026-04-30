/**
 * \file
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * \ingroup utils
 *
 * \brief Byte-level macros and functions, such as swapping and string generation.
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stddef.h>
#include <stdint.h>

#include "rcsw/core/compilers.h"
#include "rcsw/core/core.h"
#include "rcsw/utils/bit.h"

/******************************************************************************
 * Macros
 ******************************************************************************/
/** \brief Swap the byte order of a 16-bit value. */
#define RCSW_BSWAP16(w16) ((((w16) & 0xFF00) >> 8) | (((w16) & 0xFF) << 8))

/** \brief Swap the byte order of a 32-bit value. */
#define RCSW_BSWAP32(w32)                                     \
  ((((w32) & 0xFF000000) >> 24) | (((w32) & 0xFF0000) >> 8) | \
   (((w32) & 0xFF00) << 8) | (((w32) & 0xFF) << 24))

/** \brief Swap the byte order of a 64-bit value. */
#define RCSW_BSWAP64(w64)                                \
  (((uint64_t)RCSW_BSWAP32(RCSW_BITS_LO64(w64)) << 32) | \
   ((uint64_t)(RCSW_BSWAP32(RCSW_BITS_HI64(w64) >> 32))))

/** \brief Swap the two 16-bit halves of a 32-bit value (word swap). */
#define RCSW_WSWAP32(w32) \
  ((((w32) & 0xFFFF0000) >> 16) | (((w32) & 0xFFFF) << 16))

/*******************************************************************************
 * Public API
 ******************************************************************************/
BEGIN_C_DECLS
/**
 * \brief Reverse the bytes of an array in place.
 *
 * \param arr  The byte array to reverse.
 * \param size Number of bytes in the array.
 */
RCSW_API void utils_arr8_reverse(void* arr, size_t size);

/**
 * \brief Swap two elements in a uint32_t array.
 *
 * \param v Array containing the elements.
 * \param i Index of the first element.
 * \param j Index of the second element.
 */
RCSW_API void utils_elt_swap(void* arr, size_t elt_size, size_t i, size_t j);

/**
 * \brief Fill \p buf with a random printable ASCII string.
 *
 * Fills \p len - 1 characters from the printable ASCII range [33, 126]
 * and appends a null terminator, so \c strlen(buf) == len - 1 afterward.
 *
 * \param buf Buffer to fill. Must be at least \p len bytes.
 * \param len Total buffer size including the null terminator.
 *
 * \return \ref status_t
 */
RCSW_API status_t utils_string_gen(char* buf, size_t len);

END_C_DECLS
