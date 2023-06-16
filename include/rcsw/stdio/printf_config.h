/**
 * \file printf_config.h
 *
 * \author (c) Eyal Rozenberg <eyalroz1@gmx.com>
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
#include "rcsw/common/common.h"


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

#if RCSW_STDIO_PRINTF_SUPPORT_LONG_LONG
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

#define PRINTF_MAX_BUF_SIZE INT_MAX
// If we were to nitpick, this would actually be INT_MAX + 1,
// since INT_MAX is the maximum return value, which excludes the
// trailing '\0'.
