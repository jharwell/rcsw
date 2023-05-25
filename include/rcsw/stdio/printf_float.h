/**
 * \file printf_float.h
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/stdio/math.h"

#include "rcsw/stdio/printf_gadget.h"
/*******************************************************************************
 * Macros
 ******************************************************************************/
#define PRINTF_ABS(_x) ( (_x) > 0 ? (_x) : -(_x) )

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS
void print_floating_point(struct printf_output_gadget* output,
                          double value,
                          printf_size_t precision,
                          printf_size_t width,
                          printf_flags_t flags,
                          bool_t prefer_exponential);

END_C_DECLS
