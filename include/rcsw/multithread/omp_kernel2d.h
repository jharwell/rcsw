/**
 * \file omp_kernel2d.h
 * \ingroup multithread
 * \brief Convolution of 2D array with 3x3 kernel via OpenMP
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * This file is part of RCSW.
 *
 * RCSW is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * RCSW is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * RCSW.  If not, see <http://www.gnu.org/licenses/
 */

#ifndef INCLUDE_RCSW_MULTITHREAD_OMP_KERNEL2D_H_
#define INCLUDE_RCSW_MULTITHREAD_OMP_KERNEL2D_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/common/common.h"

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Apply a 3x3 kernel to a 2D array once, populating a new output array.
 *
 * \param input The input array.
 * \param stencil The kernel (must be 3x3).
 * \param width Width of 2D array.
 * \param length Length of 2D array.
 * \param output The output array.
 *
 * \return \ref status_t.
 */
status_t omp_kernel2d_convolve1(float const * const __restrict__ input,
                                float kernel[3][3],
                                size_t width, size_t height,
                                float * const __restrict__ output);
END_C_DECLS

#endif /* INCLUDE_RCSW_MULTITHREAD_OMP_KERNEL2D_H_ */
