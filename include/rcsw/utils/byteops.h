/**
 * \file
 *
 * \copyright 2017 John Harwell
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
#define RCSW_BSWAP32(w32)                    \
  ((((uint32_t)(w32) & 0xFF000000U) >> 24) | \
   (((uint32_t)(w32) & 0x00FF0000U) >> 8) |  \
   (((uint32_t)(w32) & 0x0000FF00U) << 8) |  \
   (((uint32_t)(w32) & 0x000000FFU) << 24))

/** \brief Swap the byte order of a 64-bit value. */
#define RCSW_BSWAP64(w64)                                            \
  (((uint64_t)RCSW_BSWAP32((uint64_t)(w64) & 0xFFFFFFFFU) << 32) | \
   (uint64_t)RCSW_BSWAP32((uint64_t)(w64) >> 32))

/*******************************************************************************
 * Public API
 ******************************************************************************/
BEGIN_C_DECLS
/**
 * \brief Reverse the bytes of an array in place.
 *
 * Header-only, so that \ref stdio_strrev() can use it without linking against
 * the utils module.
 *
 * \param arr  The byte array to reverse. Can be NULL if \p size is 0.
 * \param size Number of bytes in the array.
 */
static inline void utils_arr8_reverse(void* const arr, size_t size) {
  uint8_t* const a = (uint8_t*)arr;
  for (size_t i = 0, j = size; i + 1 < j; ++i) {
    --j;
    uint8_t tmp = a[i];
    a[i]        = a[j];
    a[j]        = tmp;
  } /* for() */
}

/**
 * \brief Swap two elements of any size.
 *
 * Works on raw bytes through a small fixed-size buffer, so the elements need
 * no particular alignment and there is no limit on their size. Swapping an
 * element with itself does nothing; otherwise the elements must not overlap.
 *
 * \param elt1 The first element.
 * \param elt2 The second element.
 * \param elt_size Size of each element in bytes.
 */
RCSW_API void utils_elt_swap(void* elt1, void* elt2, size_t elt_size);

/**
 * \brief Fill \p buf with a random printable ASCII string.
 *
 * Fills \p len - 1 characters from the printable ASCII range [33, 126]
 * and appends a null terminator, so \c strlen(buf) == len - 1 afterward.
 *
 * Characters come from the C library's random(). Not reentrant, and not
 * suitable for anything security-related.
 *
 * \param buf Buffer to fill. Must be at least \p len bytes.
 * \param len Total buffer size including the null terminator.
 *
 * \return \ref status_t
 */
RCSW_API status_t utils_string_gen(char* buf, size_t len);

END_C_DECLS
