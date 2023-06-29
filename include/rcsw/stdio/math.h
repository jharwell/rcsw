/**
 * \file math.h
 *
 * \author (c) Eyal Rozenberg <eyalroz@\gmx.com>
 *             2021-2022, Haifa, Palestine/Israel
 * \author (c) Marco Paland (info@paland.com)
 *             2014-2019, PALANDesign Hannover, Germany
 *
 * SPDX-License Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <float.h>
#include <limits.h>

#include "rcsw/common/common.h"

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/

#if FLT_RADIX != 2
#error "Non-binary-radix floating-point types are unsupported."
#endif

/*******************************************************************************
 * Configuration
 ******************************************************************************/
#if DBL_MANT_DIG == 24

#define DOUBLE_SIZE_IN_BITS 32
typedef uint32_t double_uint_t;
#define DOUBLE_EXPONENT_MASK 0xFFU
#define DOUBLE_BASE_EXPONENT 127
#define DOUBLE_MAX_SUBNORMAL_EXPONENT_OF_10 -38
#define DOUBLE_MAX_SUBNORMAL_POWER_OF_10 1e-38

#elif DBL_MANT_DIG == 53

#define DOUBLE_SIZE_IN_BITS 64
typedef uint64_t double_uint_t;
#define DOUBLE_EXPONENT_MASK 0x7FFU
#define DOUBLE_BASE_EXPONENT 1023
#define DOUBLE_MAX_SUBNORMAL_EXPONENT_OF_10 -308
#define DOUBLE_MAX_SUBNORMAL_POWER_OF_10 1e-308

#else
#error "Unsupported double type configuration"
#endif
#define DOUBLE_STORED_MANTISSA_BITS (DBL_MANT_DIG - 1)

typedef union {
  double_uint_t U;
  double        F;
} double_with_bit_access;


/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

// This is unnecessary in C99, since compound initializers can be used,
// but:
// 1. Some compilers are finicky about this;
// 2. Some people may want to convert this to C89;
// 3. If you try to use it as C++, only C++20 supports compound literals
static inline double_with_bit_access get_bit_access(double x) {
  double_with_bit_access dwba;
  dwba.F = x;
  return dwba;
}

static inline int stdio_sign_bit(double x)
{
  // The sign is stored in the highest bit
  return (int) (get_bit_access(x).U >> (DOUBLE_SIZE_IN_BITS - 1));
}

static inline int stdio_exp2(double_with_bit_access x)
{
  // The exponent in an IEEE-754 floating-point number occupies a contiguous
  // sequence of bits (e.g. 52..62 for 64-bit doubles), but with a non-trivial representation: An
  // unsigned offset from some negative value (with the extremal offset values reserved for
  // special use).
  return (int)((x.U >> DOUBLE_STORED_MANTISSA_BITS ) & DOUBLE_EXPONENT_MASK) - DOUBLE_BASE_EXPONENT;
}

RCSW_WARNING_DISABLE_PUSH()
RCSW_WARNING_DISABLE_FLOAT_EQUAL()
static inline bool stdio_isnan(double value) { return value != value; }
RCSW_WARNING_DISABLE_POP()

static inline bool stdio_isinf(double value) { return value < -DBL_MAX || value > DBL_MAX; }

// A floor function - but one which only works for numbers whose
// floor value is representable by an int.
int stdio_floor(double x) RCSW_CONST;

/**
 * Computes the base-10 logarithm of the input number - which must be an actual
 * positive number (not infinity or NaN, nor a sub-normal).
 */
double stdio_log10(double positive_number) RCSW_CONST;

double stdio_pow10(int floored_exp10) RCSW_CONST;

END_C_DECLS
