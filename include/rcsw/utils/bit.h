/**
 * \file
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * \ingroup utils
 *
 * \brief Bit manipulation macros (masking, reversal, reflection,
 * endianness swapping), and related helpers.
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
 * \def RCSW_BITS_HI32(v) - Get the upper 16 bits of a 32 bit integer in a 32-bit
 * integer.
 */
#define RCSW_BITS_HI32(v) ((uint32_t)((v) & 0xFFFF0000))

/**
 * \def RCSW_BITS_LO32(v) - Get the lower 16 bits of a 32 bit integer.
 */
#define RCSW_BITS_LO32(v) ((uint32_t)((v) & 0x0000FFFF))

/**
 * \def RCSW_BITS_HI64(v) - Get the upper 32 bits of a 64 bit integer as a 64-bit
 * integer.
 */
#define RCSW_BITS_HI64(v) ((uint64_t)(((v) & UINT64_C(0xFFFFFFFF00000000))))

/**
 * \def RCSW_BITS_LO64(v) - Get the lower 32 bits of a 64 bit integer.
 */
#define RCSW_BITS_LO64(v) (((uint64_t)((v) & 0x00000000FFFFFFFF)))

/**
 * Reversal macros (MSB becomes LSB and vice versa) This is NOT the same as
 * endianness swapping.
 */

/** Reversal using bit shifting */
#define RCSW_REV8(v)                                                          \
  ((uint8_t)(((((v) * 0x0802LU & 0x22110LU) | ((v) * 0x8020LU & 0x88440LU)) * \
                0x10101LU >>                                                  \
              16)))
#define RCSW_REV16(v) \
  ((uint16_t)((RCSW_REV8(((v) & 0xFF)) << 8) | RCSW_REV8((((v) >> 8) & 0xFF))))
#define RCSW_REV32(v)                                \
  ((uint32_t)(((RCSW_REV16(((v) & 0xFFFF))) << 16) | \
              (RCSW_REV16((((v) >> 16) & 0xFFFF)))))

/** Reversal using a lookup table -- \ref rcsw_util_revtable */
#define RCSW_REVTBL8(v) ((uint8_t)(rcsw_util_revtable[v]))
#define RCSW_REVTBL16(v)                          \
  ((uint16_t)((RCSW_REVTBL8(((v) & 0xFF)) << 8) | \
              RCSW_REVTBL8((((v) >> 8) & 0xFF))))
#define RCSW_REVTBL32(v)                                \
  ((uint32_t)(((RCSW_REVTBL16(((v) & 0xFFFF))) << 16) | \
              (RCSW_REVTBL16((((v) >> 16) & 0xFFFF)))))

/** Bit reflection (reflect/mirror an 8,16, or 32 bit value about its center)  */
#define RCSW_REFLECT8(v) ((uint8_t)utils_reflect32(((v)), 8))
#define RCSW_REFLECT16(v) ((uint16_t)utils_reflect32(((v)), 16))
#define RCSW_REFLECT32(v) ((uint32_t)utils_reflect32(((v)), 32))

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
 * \defgroup rcsw_bin Binary-to-hex Conversion
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

#define RCSW_BIN16(b1, b0)                                      \
  (((uint8_t)RCSW_BIN_DECODE(RCSW_BIN_MAKE_LITERAL(b1)) << 8) | \
   (uint8_t)RCSW_BIN_DECODE(RCSW_BIN_MAKE_LITERAL(b0)))

#define RCSW_BIN32(b3, b2, b1, b0)                               \
  (((uint8_t)RCSW_BIN_DECODE(RCSW_BIN_MAKE_LITERAL(b3)) << 24) | \
   ((uint8_t)RCSW_BIN_DECODE(RCSW_BIN_MAKE_LITERAL(b2)) << 16) | \
   ((uint8_t)RCSW_BIN_DECODE(RCSW_BIN_MAKE_LITERAL(b1)) << 8) |  \
   (uint8_t)RCSW_BIN_DECODE(RCSW_BIN_MAKE_LITERAL(b0)))

/** @} */

/**
 * \brief Width in bits of type \p t. A char is 8 bits an almost every
 * platform, but for some DSPs, it can be something different.
 */
#define RCSW_BIT_WIDTH(t) (sizeof(t) * CHAR_BIT)

/** \brief Value with only the top bit of type \p t set. */
#define RCSW_TOPBIT(t) (1U << (RCSW_BIT_WIDTH(t) - 1))

/** Generate a 32-bit bitmask based on \p shift */
#define RCSW_GEN_M32(name, shift) name = (1U << (shift))

/** Generate a 64-bit bitmask based on \p shift */
#define RCSW_GEN_M64(name, shift) name = (1ULL << (shift))

/******************************************************************************
 * Global Variables
 ******************************************************************************/
BEGIN_C_DECLS

/** \brief Lookup table for 8-bit bit-reversal. Used by \ref RCSW_REVL8. */
RCSW_API extern const uint8_t rcsw_util_revtable[];

/******************************************************************************
 * Public API
 ******************************************************************************/
/**
 * \brief Reflect the lowest \p n_bits bits of \p data about their center.
 *
 * Bit 0 swaps with bit n_bits-1, bit 1 with bit n_bits-2, and so on.
 * Used internally by CRC computation routines.
 *
 * \param data   The value whose bits are to be reflected.
 * \param n_bits Number of bits to reflect (counted from LSB).
 *
 * \return The reflected value.
 */
RCSW_API uint32_t utils_reflect32(uint32_t data, size_t n_bits) RCSW_CONST;

END_C_DECLS
