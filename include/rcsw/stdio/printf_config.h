/**
 * \file printf_config.h
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/common/common.h"

/*******************************************************************************
 * Configuration
 ******************************************************************************/
// 'ntoa' conversion buffer size, this must be big enough to hold one converted
// numeric number including padded zeros (dynamically created on stack)
#ifndef PRINTF_INTEGER_BUFFER_SIZE
#define PRINTF_INTEGER_BUFFER_SIZE    32
#endif

// size of the fixed (on-stack) buffer for printing individual decimal numbers.
// this must be big enough to hold one converted floating-point value including
// padded zeros.
#ifndef PRINTF_DECIMAL_BUFFER_SIZE
#define PRINTF_DECIMAL_BUFFER_SIZE    32
#endif

// Support for the decimal notation floating point conversion specifiers (%f, %F)
#ifndef PRINTF_SUPPORT_DECIMAL_SPECIFIERS
#define PRINTF_SUPPORT_DECIMAL_SPECIFIERS 1
#endif

// Support for the exponential notation floating point conversion specifiers (%e, %g, %E, %G)
#ifndef PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS
#define PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS 1
#endif

// Support for the length write-back specifier (%n)
#ifndef PRINTF_SUPPORT_WRITEBACK_SPECIFIER
#define PRINTF_SUPPORT_WRITEBACK_SPECIFIER 1
#endif

// Default precision for the floating point conversion specifiers (the C standard sets this at 6)
#ifndef PRINTF_DEFAULT_FLOAT_PRECISION
#define PRINTF_DEFAULT_FLOAT_PRECISION  6
#endif

// According to the C languages standard, printf() and related functions must be
// able to print any integral number in floating-point notation, regardless of
// length, when using the %f specifier - possibly hundreds of characters,
// potentially overflowing your buffers. In this implementation, all values
// beyond this threshold are switched to exponential notation.
#ifndef PRINTF_MAX_INTEGRAL_DIGITS_FOR_DECIMAL
#define PRINTF_MAX_INTEGRAL_DIGITS_FOR_DECIMAL 9
#endif

// Support for the long long integral types (with the ll, z and t length
// modifiers for specifiers %d,%i,%o,%x,%X,%u, and with the %p specifier). Note:
// 'L' (long double) is not supported.
#ifndef PRINTF_SUPPORT_LONG_LONG
#define PRINTF_SUPPORT_LONG_LONG 1
#endif

// The number of terms in a Taylor series expansion of log_10(x) to
// use for approximation - including the power-zero term (i.e. the
// value at the point of expansion).
#ifndef PRINTF_LOG10_TAYLOR_TERMS
#define PRINTF_LOG10_TAYLOR_TERMS 4
#endif

#if PRINTF_LOG10_TAYLOR_TERMS <= 1
#error "At least one non-constant Taylor expansion is necessary for the log10() calculation"
#endif

// Be extra-safe, and don't assume format specifiers are completed correctly
// before the format string end.
#ifndef PRINTF_CHECK_FOR_NUL_IN_FORMAT_SPECIFIER
#define PRINTF_CHECK_FOR_NUL_IN_FORMAT_SPECIFIER 1
#endif

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
#define PRINTF_PREFER_DECIMAL     FALSE
#define PRINTF_PREFER_EXPONENTIAL TRUE

// internal flag definitions
#define FLAGS_ZEROPAD   (1U <<  0U)
#define FLAGS_LEFT      (1U <<  1U)
#define FLAGS_PLUS      (1U <<  2U)
#define FLAGS_SPACE     (1U <<  3U)
#define FLAGS_HASH      (1U <<  4U)
#define FLAGS_UPPERCASE (1U <<  5U)
#define FLAGS_CHAR      (1U <<  6U)
#define FLAGS_SHORT     (1U <<  7U)
#define FLAGS_INT       (1U <<  8U)
#define FLAGS_LONG      (1U <<  9U)
#define FLAGS_LONG_LONG (1U << 10U)
#define FLAGS_PRECISION (1U << 11U)
#define FLAGS_ADAPT_EXP (1U << 12U)
#define FLAGS_POINTER   (1U << 13U)
// Note: Similar, but not identical, effect as FLAGS_HASH
#define FLAGS_SIGNED    (1U << 14U)

#define BASE_BINARY    2
#define BASE_OCTAL     8
#define BASE_DECIMAL  10
#define BASE_HEX      16


typedef unsigned int printf_flags_t;


typedef uint8_t numeric_base_t;

#if PRINTF_SUPPORT_LONG_LONG
typedef unsigned long long printf_unsigned_value_t;
typedef long long          printf_signed_value_t;
#else
typedef unsigned long printf_unsigned_value_t;
typedef long          printf_signed_value_t;
#endif

// The printf()-family functions return an `int`; it is therefore
// unnecessary/inappropriate to use size_t - often larger than int
// in practice - for non-negative related values, such as widths,
// precisions, offsets into buffers used for printing and the sizes
// of these buffers. instead, we use:
typedef unsigned int printf_size_t;

#define PRINTF_MAX_POSSIBLE_BUFFER_SIZE INT_MAX
// If we were to nitpick, this would actually be INT_MAX + 1,
// since INT_MAX is the maximum return value, which excludes the
// trailing '\0'.
