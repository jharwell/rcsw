/**
 * \file printf_float.h
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
#include "rcsw/stdio/math.h"
#include "rcsw/stdio/printf_gadget.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/
/* \cond INTERNAL */
#define PRINTF_ABS(_x) ( (_x) > 0 ? (_x) : -(_x) )

/*******************************************************************************
 * RCSW Private Functions
 ******************************************************************************/
BEGIN_C_DECLS
RCSW_LOCAL void print_floating_point(struct printf_output_gadget* output,
                          double value,
                          printf_size_t precision,
                          printf_size_t width,
                          printf_flags_t flags,
                          bool_t prefer_exponential);

END_C_DECLS

/* \endcond */
