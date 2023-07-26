/**
 * \file printf_internal.h
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

#include "rcsw/stdio/printf_config.h"
#include "rcsw/stdio/printf_gadget.h"
#if (RCSW_STDIO_PRINTF_SUPPORT_DEC || RCSW_STDIO_PRINTF_SUPPORT_EXP)
#include "rcsw/stdio/printf_float.h"
#endif

/*******************************************************************************
 * Macros
 ******************************************************************************/
/* \cond INTERNAL */
/*
 * Note in particular the behavior here on LONG_MIN or LLONG_MIN; it is valid
 * and well-defined, but if you're not careful you can easily trigger undefined
 * behavior with -LONG_MIN or -LLONG_MIN/
 */
#define ABS_FOR_PRINTING(_x) \
  ((printf_unsigned_value_t) ( (_x) > 0 ? (_x) : -((printf_signed_value_t)_x) ))


/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * Output the specified string in reverse, taking care of any zero-padding
 */
void out_reversed(struct printf_output_gadget* output,
                  const char* buf, printf_size_t len,
                  printf_size_t width,
                  printf_flags_t flags);

/**
 * An internal itoa-like function
 */
void print_integer(struct printf_output_gadget* output,
                   printf_unsigned_value_t value,
                   bool_t negative,
                   numeric_base_t base,
                   printf_size_t precision,
                   printf_size_t width,
                   printf_flags_t flags);

/**
 * Invoked by print_integer after the actual number has been printed, performing
 * necessary work on the number's prefix (as the number is initially printed in
 * reverse order)
 */
void print_integer_finalization(struct printf_output_gadget* output,
                                char* buf,
                                printf_size_t len,
                                bool_t negative,
                                numeric_base_t base,
                                printf_size_t precision,
                                printf_size_t width,
                                printf_flags_t flags);

/* \endcond */

END_C_DECLS
