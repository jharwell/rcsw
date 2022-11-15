/**
 * \file simple_image.h
 * \ingroup pulse
 * \brief Implementation of adapter design pattern for STB.
 * Originally intended to make GPU programming in C easier.
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

#ifndef INCLUDE_RCSW_ADAPTER_SIMPLE_IMAGE_H_
#define INCLUDE_RCSW_ADAPTER_SIMPLE_IMAGE_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdlib.h>
#include "rcsw/common/common.h"

/*******************************************************************************
 * Type Definitions
 ******************************************************************************/
/**
 * \brief A structure representing an RGB image. Pixels are stored row-major.
 */
typedef struct {
    int width;   /// The width of the image in pixels.
    int height;  /// The height of the image in pixels.

    /*
     * Actual image data. These arrays are all of length (width x height). All
     * arrays are row-major, and must NOT overlap.
     */
    float *__restrict__ red;
    float *__restrict__ green;
    float *__restrict__ blue;
} simple_image_t;

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Count the number of FLOPs performed during an arbitrary stencil
 *        application.
 *
 * \param image       The image we are evaluating.
 * \param kernel_dim The number of rows or columns in the stencil.
 *
 * \return The number of FLOPs.
 */
size_t simple_image_kernel2d_flops(simple_image_t const * image,
                                size_t kernel_dim) RCSW_PURE;

/**
 * \brief Load an image from a file. The image can be JPEG, PNG, or BMP.
 *
 * \param filename The name of the file to load.
 *
 * \return  The loaded image, or NULL on error.
 */
simple_image_t * simple_image_load(char const * filename);

/**
 * \brief Some pixels can escape the [0, 255] range. This maps all values back
 *        into the valid range.
 *
 * \param image The image to scale.
 *
 * \return \ref status_t.
 */
status_t simple_image_clamp_rgb(simple_image_t * image);

/**
 * \brief Write an image to a bitmap file. Note that this will map the pixels
 *        back to a valid range if necessary, so the image data *may* change.
 *
 * \param filename The name of the file to create.
 * \param image The image structure to write.
 *
 * \return \ref status_t.
 */
status_t simple_image_write_bmp(char const * filename,
                                simple_image_t * image);

/**
 * \brief Allocate an image.
 *
 * \param width The width, in pixels.
 * \param height The height, in pixels.
 */
simple_image_t * simple_image_alloc(size_t width, size_t height);

/**
 * \brief Free the memory allocated by \ref simple_image_load().
 *
 * \param im The image to free.
 */
void simple_image_free(simple_image_t * im);

END_C_DECLS

#endif /* INCLUDE_RCSW_ADAPTER_SIMPLE_IMAGE_H_ */
