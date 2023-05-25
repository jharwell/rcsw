/**
 * \file printf_internal.h
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

#include "rcsw/stdio/printf_config.h"
#include "rcsw/stdio/printf_gadget.h"
#if (PRINTF_SUPPORT_DECIMAL_SPECIFIERS || PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS)
#include "rcsw/stdio/printf_float.h"
#endif

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/

/*******************************************************************************
 * Macros
 ******************************************************************************/
// Note in particular the behavior here on LONG_MIN or LLONG_MIN; it is valid
// and well-defined, but if you're not careful you can easily trigger undefined
// behavior with -LONG_MIN or -LLONG_MIN
#define ABS_FOR_PRINTING(_x) ((printf_unsigned_value_t) ( (_x) > 0 ? (_x) : -((printf_signed_value_t)_x) ))


/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS
void out_rev_(struct printf_output_gadget* output,
              const char* buf, printf_size_t len, printf_size_t width, printf_flags_t flags);

void print_integer(struct printf_output_gadget* output, printf_unsigned_value_t value, bool_t negative, numeric_base_t base, printf_size_t precision, printf_size_t width, printf_flags_t flags);

void print_integer_finalization(
    struct printf_output_gadget* output, char* buf, printf_size_t len, bool_t negative, numeric_base_t base, printf_size_t precision, printf_size_t width, printf_flags_t flags);

END_C_DECLS
