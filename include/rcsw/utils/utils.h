/**
 * \file utils.h
 * \ingroup utils
 * \brief Miscellaneous utility functions/macros/definitions/etc.
 *
 * A catch all for random things I found interesting that didn't fit anywhere
 * else.
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/rcsw.h"
#include "rcsw/common/fpc.h"

/*******************************************************************************
 * Bit Manipulation Macros
 ******************************************************************************/
/**
 * \def RCSW_M32U16(v) - Get the upper 16 bits of a 32 bit integer in a 32-bit
 * integer.
 */
#define RCSW_M32U16(v) ((uint32_t)((v) & 0xFFFF0000))

/**
 * \def RCSW_M32L16(v) - Get the lower 16 bits of a 32 bit integer.
 */
#define RCSW_M32L16(v) ((uint32_t)((v) & 0x0000FFFF))

/**
 * \def RCSW_M64U32(v) - Get the upper 32 bits of a 64 bit integer as a 64-bit
 * integer.
 */
#define RCSW_M64U32(v) ((uint64_t)(((v) & 0xFFFFFFFF00000000)))

/**
 * \def RCSW_M64L32(v) - Get the lower 32 bits of a 64 bit integer.
 */
#define RCSW_M64L32(v) (((uint64_t)((v) & 0x00000000FFFFFFFF)))

/**
 * Reversal macros (MSB becomes LSB and vice versa) This is NOT the same as
 * endianness swapping.
 */

/** Reversal using bit shifting */
#define RCSW_REV8(v)                                                         \
    ((uint8_t)(                                                         \
        ((((v) * 0x0802LU & 0x22110LU) | ((v) * 0x8020LU & 0x88440LU)) * \
         0x10101LU >>                                                   \
         16)))
#define RCSW_REV16(v) ((uint16_t)((RCSW_REV8(((v) & 0xFF)) << 8) | \
                                  RCSW_REV8((((v) >> 8) & 0xFF))))
#define RCSW_REV32(v)((uint32_t)(((RCSW_REV16(((v) & 0xFFFF))) << 16) | \
                                 (RCSW_REV16((((v) >> 16) & 0xFFFF)))))

/** Reversal using a lookup table */
#define RCSW_REVL8(v) ((uint8_t)(rcsw_util_revtable[v]))
#define RCSW_REVL16(v) ((uint16_t)((RCSW_REVL8(((v) & 0xFF)) << 8) | \
                                   RCSW_REVL8((((v) >> 8) & 0xFF))))
#define RCSW_REVL32(v) ((uint32_t)(((RCSW_REVL16(((v) & 0xFFFF))) << 16) | \
                                   (RCSW_REVL16((((v) >> 16) & 0xFFFF)))))

/** Bit reflection (reflect/mirror an 8,16, or 32 bit value about its center)  */
#define RCSW_REFL8(v) ((uint8_t)reflect(((v)), 8))
#define RCSW_REFL16(v) ((uint16_t)reflect(((v)), 16))
#define RCSW_REFL32(v) ((uint32_t)reflect(((v)), 32))

/**
 * Convert 8-bit binary to hexadecimal.
 */
#define RCSW_BIN8(b) (RCSW_BIN8_HEXIFY((uint8_t)RCSW_BIN8_HEXIFY(b)))

/**
 * Convert 16-bit binary to hexadecimal.
 */
#define RCSW_BIN16(b1, b0)                                              \
  (((BIN8_HEXIFY((uint8_t)RCSW_BIN8_HEXIFY(b1))) << 8) + RCSW_HEXIFY(b0))

/**
 * Convert 32-bit binary to hexadecimal.
 */
#define RCSW_BIN32(b3, b4, b1, b0)                                      \
  ((((RCSW_BIN8_HEXIFY((uint8_t)RCSW_BIN8_HEXIFY(b4))) << 8) +          \
    RCSW_BIN8(b3))(((RCSW_BIN8_HEXIFY((uint8_t)RCSW_BIN8_HEXIFY(b1))) << 8) + RCSW_BIN8(b0)))

/* \cond INTERNAL */
#define RCSW_HEXIFY (n)(0x##n##LU)

#define RCSW_BIN8_HEXIFY(x)                                             \
  ((((x) & 0x0000000FLU) ? 1 : 0) + (((x) & 0x000000F0LU) ? 2 : 0) +    \
   (((x) & 0x00000F00LU) ? 4 : 0) + (((x) & 0x0000F000LU) ? 8 : 0) +    \
   (((x) & 0x000F0000LU) ? 16 : 0) + (((x) & 0x00F00000LU) ? 32 : 0) +  \
   (((x) & 0x0F000000LU) ? 64 : 0) + (((x) & 0xF0000000LU) ? 128 : 0))

/* \endcond */


/** Miscellaneous bit manipulation */
#define RCSW_BIT_WIDTH(t) (sizeof(t) * 8)
#define RCSW_TOPBIT(t) (1 << (BIT_WIDTH(t) - 1))

#define RCSW_GEN_M32(name, num) name = (1 << (num)),
#define RCSW_GEN_M64(name, num) name = ((long long int)1 << (num)),

/*******************************************************************************
 * Alignment Macros
 ******************************************************************************/
/** Pointer alignment checking */
#define RCSW_IS_MEM_ALIGNED(p, byte_count) ((((uintptr_t)(p)) % (byte_count)) == 0)

/** Size alignment checking */
#define RCSW_IS_SIZE_ALIGNED(size, power_of_two) (((size) & ((power_of_two)-1)) == 0)

/** Align a requested size to the requested power of 2  */
#define RCSW_ALIGN_SIZE(size, power_of_two)                  \
    (((size) + (power_of_two)-1) & ~((power_of_two)-1))

/*******************************************************************************
 * Endianness Macros
 ******************************************************************************/
/**
 * \brief Test if the architecture is little endian without relying on compiler.
 *
 * Cons: run-time determination.
 */
#define RCSW_IS_LITTLE_ENDIAN() (((*(short *)"21") & 0xFF) == '2')

/**
 * Test if the architecture is big endian without relying on compiler macros.
 *
 * Cons: run-time determination.
 */
#define RCSW_IS_BIG_ENDIAN() (((*(short *)"21") & 0xFF) == '1')

/** Explicitly change the endianness of a 16-bit number. */
#define RCSW_BSWAP16(w16) ((((w16)&0xFF00) >> 8) | (((w16)&0xFF) << 8))

/** Explicitly change the endiannes of a 32-bit number. */
#define RCSW_BSWAP32(w32)                                            \
    ((((w32)&0xFF000000) >> 24) | (((w32)&0xFF0000) >> 8) |     \
     (((w32)&0xFF00) << 8) | (((w32)&0xFF) << 24))

/** Explicit change the endianness of a 64-bit number */
#define RCSW_BSWAP64(w64)                                                    \
    (((uint64_t)RCSW_BSWAP32(RCSW_M64L32(w64)) << 32) | \
     ((uint64_t)(RCSW_BSWAP32(RCSW_M64U32(w64) >> 32))))

/** Not an endianness change, but still useful sometimes */
#define RCSW_BSWAP32_16(w32) ((((w32)&0xFFFF0000) >> 16) | (((w32)&0xFFFF) << 16))

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
BEGIN_C_DECLS

RCSW_API extern const uint8_t rcsw_util_revtable[];

/*******************************************************************************
 * API Functions
 ******************************************************************************/
/**
 * \brief Ensure a value is in the range [0, 255]. Value may be nearby after
 *        scaling due to floating point rounding.
 *
 * \param v The value to round.
 *
 * \return The rounded value.
 */
RCSW_PURE static inline float utils_clamp_f255(float v) {
    if (v < 0) {
        return 0.0F;
    }
    if (v > 255.0F) {
      return 255.0F;
    }
    return v;
} /* utils_clam_f255() */

/**
 * \brief Reverse the bytes an array.
 *
 * \param arr The byte array to reverse.
 *
 * \param size # of bytes in array.
 *
 */
RCSW_API void arr8_reverse(void* arr, size_t size);

/**
 * \brief Generate permutations from elements within an array.
 *
 * From element "start" to element "size" - 1.
 *
 * \param arr The array of integers to permute.
 * \param size # elements in array.
 * \param start Start position in array to permute
 * \param fp A callback which each permutation is handed to in turn.
 */
RCSW_API void arr32_permute(uint32_t *arr, size_t size, size_t start,
                            void (*fp)(uint32_t * elt));

/**
 * \brief Swap two 32-bit elements in an array.
 *
 * \param v The array
 * \param i Index #1
 * \param j Index #2
 */
RCSW_API void arr32_elt_swap(uint32_t * v, size_t i, size_t j);

/**
 * \brief Generate a random alpha-numeric string of known length.
 *
 * Buf is filled with len - 1 alpha-numeric characters and a NULL byte at the
 * end such that strlen(buf) = len -1 after this function executes.
 *
 * \param buf The buffer to fill.
 * \param len The # of characters to put in the string, -1 for the NULL byte.
 *
 * \return \ref status_t.
 */
RCSW_API status_t util_string_gen(char * buf, size_t len);

/**
 *
 * \brief Reflect N bits about the center position.
 *
 * Starting from the LSB up to \p n_bits, reflect around the dividing line
 * between bits 14-15.
 *
 * \param data The data to reflect.
 * \param n_bits # of bits to reflect.
 *
 * \return The reflected data.
 */
RCSW_API uint32_t util_reflect32(uint32_t data, size_t n_bits) RCSW_CONST;

/**
 * \brief Utility function to check if an element is 0.
 *
 * If the element is larger than double, a for() loop is used. Otherwise
 * pointers are used.
 *
 * \param elt Element to check.
 * \param elt_size Size of element in bytes.
 *
 * \return \ref bool_t
 */
RCSW_API bool_t util_zchk(void *elt, size_t elt_size);

END_C_DECLS
