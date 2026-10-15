/**
 * \file
 *
 * \copyright 2017 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * \ingroup utils
 *
 * \brief Bit manipulation macros (masking, binary literals, widths).
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <limits.h>
#include <stddef.h>
#include <stdint.h>

#include "rcsw/core/compilers.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/
/**
 * \brief Width in bits of type \p t. A char is 8 bits an almost every
 * platform, but for some DSPs, it can be something different.
 */
#define RCSW_BIT_WIDTH(t) (sizeof(t) * CHAR_BIT)

/**
 * \brief Value with only the top bit of type \p t set, as a \c uintmax_t.
 *
 * Works for any integer type up to the width of \c uintmax_t (a plain
 * \c 1U shift is undefined for types wider than \c unsigned \c int).
 */
#define RCSW_TOPBIT(t) ((uintmax_t)1 << (RCSW_BIT_WIDTH(t) - 1))

/** Generate a 32-bit bitmask based on \p shift */
#define RCSW_GEN_M32(name, shift) name = (1U << (shift))

/** Generate a 64-bit bitmask based on \p shift */
#define RCSW_GEN_M64(name, shift) name = (1ULL << (shift))
