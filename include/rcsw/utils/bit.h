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
 * \brief Mask off all but the upper 16 bits of the 32-bit integer \p v. The
 * bits stay in place; they are not shifted down.
 */
#define RCSW_BITS_HI32(v) ((uint32_t)((v) & 0xFFFF0000))

/**
 * \brief Mask off all but the lower 16 bits of the 32-bit integer \p v.
 */
#define RCSW_BITS_LO32(v) ((uint32_t)((v) & 0x0000FFFF))

/**
 * \brief Mask off all but the upper 32 bits of the 64-bit integer \p v. The
 * bits stay in place; they are not shifted down.
 */
#define RCSW_BITS_HI64(v) ((uint64_t)(((v) & UINT64_C(0xFFFFFFFF00000000))))

/**
 * \brief Mask off all but the lower 32 bits of the 64-bit integer \p v.
 */
#define RCSW_BITS_LO64(v) (((uint64_t)((v) & 0x00000000FFFFFFFF)))

/* \cond INTERNAL */

/*
 * Prepend 0x and append LU to a token, forming an unsigned long literal.
 * The token must already be written in the pseudo-binary notation used by
 * RCSW_BIN8 (e.g. 11001100 becomes 0x11001100LU).
 */
#define RCSW_BIN_MAKE_LITERAL(n) (0x##n##LU)

/*
 * Convert a pseudo-binary integer (where each hex digit represents one bit)
 * to its true integer value. E.g. 0x11001100LU -> 204.
 */
#define RCSW_BIN_DECODE(x)                                             \
  ((((x) & 0x0000000FLU) ? 1 : 0) + (((x) & 0x000000F0LU) ? 2 : 0) +   \
   (((x) & 0x00000F00LU) ? 4 : 0) + (((x) & 0x0000F000LU) ? 8 : 0) +   \
   (((x) & 0x000F0000LU) ? 16 : 0) + (((x) & 0x00F00000LU) ? 32 : 0) + \
   (((x) & 0x0F000000LU) ? 64 : 0) + (((x) & 0xF0000000LU) ? 128 : 0))

/* \endcond */

/**
 * \defgroup rcsw_bin Binary literals
 * \ingroup utils
 * Convert binary literals to their hexadecimal equivalents at
 * compile time.
 * @{
 */

/*
 * The (uint8_t) cast truncates any accidental overflow if the user passes
 * more than 8 binary digits. For well-formed inputs the outer RCSW_BIN_DECODE
 * is a no-op since the cast result is already the correct integer value.
 */
#define RCSW_BIN8(b) ((uint8_t)RCSW_BIN_DECODE(RCSW_BIN_MAKE_LITERAL(b)))

/*
 * Each byte is widened to the unsigned result type before shifting: uint8_t
 * promotes to (signed) int, and shifting a set top bit into int's sign bit is
 * undefined.
 */
#define RCSW_BIN16(b1, b0)                                                  \
  ((uint16_t)(((uint16_t)(uint8_t)RCSW_BIN_DECODE(RCSW_BIN_MAKE_LITERAL(b1)) \
               << 8) |                                                      \
              (uint16_t)(uint8_t)RCSW_BIN_DECODE(RCSW_BIN_MAKE_LITERAL(b0))))

#define RCSW_BIN32(b3, b2, b1, b0)                                         \
  (((uint32_t)(uint8_t)RCSW_BIN_DECODE(RCSW_BIN_MAKE_LITERAL(b3)) << 24) | \
   ((uint32_t)(uint8_t)RCSW_BIN_DECODE(RCSW_BIN_MAKE_LITERAL(b2)) << 16) | \
   ((uint32_t)(uint8_t)RCSW_BIN_DECODE(RCSW_BIN_MAKE_LITERAL(b1)) << 8) |  \
   (uint32_t)(uint8_t)RCSW_BIN_DECODE(RCSW_BIN_MAKE_LITERAL(b0)))

/** @} */

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
