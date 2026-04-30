/**
 * \file
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * \ingroup multithread
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stddef.h>

#include "rcsw/core/compilers.h"
#include "rcsw/core/core.h"

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Apply a 3x3 kernel to a 2D array once, populating a new output array.
 *
 * \param input The input array.
 * \param kernel The kernel (must be 3x3).
 * \param width Width of 2D array.
 * \param height Length of 2D array.
 * \param output The output array.
 *
 * \return \ref status_t.
 */
RCSW_API status_t omp_kernel2d_convolve1(float const* const __restrict__ input,
                                         float  kernel[3][3],
                                         size_t width,
                                         size_t height,
                                         float* const __restrict__ output);
END_C_DECLS
