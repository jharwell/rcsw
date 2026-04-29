/**
 * \file utils.h
 * \ingroup utils
 * \brief Miscellaneous utility functions, macros, and bit manipulation
 * helpers.
 *
 * Provides: bit manipulation macros (masking, reversal, reflection,
 * endianness swapping), alignment macros, array utilities, and small
 * helper functions that do not belong to a more specific module.
 *
 * All functions and macros are available on both POSIX and baremetal
 * targets unless noted otherwise.
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
 * \defgroup rcsw_bin Binary-to-hex Conversion
 * Convert binary literals to their hexadecimal equivalents at
 * compile time.
 * @{
 */
#define RCSW_BIN8(b)   (RCSW_BIN8_HEXIFY((uint8_t)RCSW_BIN8_HEXIFY(b)))
#define RCSW_BIN16(b1, b0)                                                   \
  (((BIN8_HEXIFY((uint8_t)RCSW_BIN8_HEXIFY(b1))) << 8) + RCSW_HEXIFY(b0))
#define RCSW_BIN32(b3, b4, b1, b0)                                           \
  ((((RCSW_BIN8_HEXIFY((uint8_t)RCSW_BIN8_HEXIFY(b4))) << 8) +              \
    RCSW_BIN8(b3))(((RCSW_BIN8_HEXIFY((uint8_t)RCSW_BIN8_HEXIFY(b1))) << 8) \
    + RCSW_BIN8(b0)))
/** @} */

/* \cond INTERNAL */
#define RCSW_HEXIFY (n)(0x##n##LU)

#define RCSW_BIN8_HEXIFY(x)                                             \
  ((((x) & 0x0000000FLU) ? 1 : 0) + (((x) & 0x000000F0LU) ? 2 : 0) +    \
   (((x) & 0x00000F00LU) ? 4 : 0) + (((x) & 0x0000F000LU) ? 8 : 0) +    \
   (((x) & 0x000F0000LU) ? 16 : 0) + (((x) & 0x00F00000LU) ? 32 : 0) +  \
   (((x) & 0x0F000000LU) ? 64 : 0) + (((x) & 0xF0000000LU) ? 128 : 0))

/* \endcond */

/** \brief Width in bits of type \p t. */
#define RCSW_BIT_WIDTH(t) (sizeof(t) * 8)

/** \brief Value with only the top bit of type \p t set. */
#define RCSW_TOPBIT(t) (1 << (BIT_WIDTH(t) - 1))

#define RCSW_GEN_M32(name, num) name = (1 << (num)),
#define RCSW_GEN_M64(name, num) name = ((long long int)1 << (num)),

/*******************************************************************************
 * Alignment Macros
 ******************************************************************************/
/** \brief True if pointer \p p is aligned to \p byte_count bytes. */
#define RCSW_IS_MEM_ALIGNED(p, byte_count) \
    ((((uintptr_t)(p)) % (byte_count)) == 0)

/** \brief True if \p size is a multiple of \p power_of_two. */
#define RCSW_IS_SIZE_ALIGNED(size, power_of_two) \
    (((size) & ((power_of_two) - 1)) == 0)

/** \brief Round \p size up to the next multiple of \p power_of_two. */
#define RCSW_ALIGN_SIZE(size, power_of_two) \
    (((size) + (power_of_two) - 1) & ~((power_of_two) - 1))

/*******************************************************************************
 * Endianness Macros
 ******************************************************************************/
/**
 * \brief Test for little-endian architecture at runtime.
 *
 * \note This is a run-time check, not a compile-time constant.
 */
#define RCSW_IS_LITTLE_ENDIAN() (((*(short *)"21") & 0xFF) == '2')

/**
 * \brief Test for big-endian architecture at runtime.
 *
 * \note This is a run-time check, not a compile-time constant.
 */
#define RCSW_IS_BIG_ENDIAN() (((*(short *)"21") & 0xFF) == '1')

/** \brief Swap the byte order of a 16-bit value. */
#define RCSW_BSWAP16(w16) ((((w16) & 0xFF00) >> 8) | (((w16) & 0xFF) << 8))

/** \brief Swap the byte order of a 32-bit value. */
#define RCSW_BSWAP32(w32)                                                    \
    ((((w32) & 0xFF000000) >> 24) | (((w32) & 0xFF0000) >> 8) |             \
     (((w32) & 0xFF00) << 8)      | (((w32) & 0xFF) << 24))

/** \brief Swap the byte order of a 64-bit value. */
#define RCSW_BSWAP64(w64)                                                    \
    (((uint64_t)RCSW_BSWAP32(RCSW_M64L32(w64)) << 32) |                     \
     ((uint64_t)(RCSW_BSWAP32(RCSW_M64U32(w64) >> 32))))

/** \brief Swap the two 16-bit halves of a 32-bit value (not a byte swap). */
#define RCSW_BSWAP32_16(w32) \
    ((((w32) & 0xFFFF0000) >> 16) | (((w32) & 0xFFFF) << 16))

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
BEGIN_C_DECLS

/** \brief Lookup table for 8-bit bit-reversal. Used by \ref RCSW_REVL8. */
RCSW_API extern const uint8_t rcsw_util_revtable[];

/*******************************************************************************
 * API Functions
 ******************************************************************************/
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
RCSW_PURE static inline float utils_clamp_f255(float v) {
    if (v < 0) {
        return 0.0F;
    }
    if (v > 255.0F) {
        return 255.0F;
    }
    return v;
}

/**
 * \brief Reverse the bytes of an array in place.
 *
 * \param arr  The byte array to reverse.
 * \param size Number of bytes in the array.
 */
RCSW_API void arr8_reverse(void* arr, size_t size);

/**
 * \brief Generate all permutations of elements [start, size) in \p arr.
 *
 * \p fp is called once with each permutation.
 *
 * \param arr   Array of uint32_t elements to permute.
 * \param size  Total number of elements in \p arr.
 * \param start Index from which to begin permuting.
 * \param fp    Callback invoked for each permutation.
 */
RCSW_API void arr32_permute(uint32_t *arr, size_t size, size_t start,
                            void (*fp)(uint32_t * elt));

/**
 * \brief Swap two elements in a uint32_t array.
 *
 * \param v Array containing the elements.
 * \param i Index of the first element.
 * \param j Index of the second element.
 */
RCSW_API void arr32_elt_swap(uint32_t * v, size_t i, size_t j);

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
RCSW_API status_t util_string_gen(char * buf, size_t len);

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
RCSW_API uint32_t util_reflect32(uint32_t data, size_t n_bits) RCSW_CONST;

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
RCSW_API bool_t util_zchk(void *elt, size_t elt_size);

END_C_DECLS
