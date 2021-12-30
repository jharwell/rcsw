/**
 * @file simple_image.c
 *
 * @copyright 2017 John Harwell, All rights reserved.
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

/*******************************************************************************
 * Includes
 ******************************************************************************/
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>

#include "rcsw/adapter/simple_image.h"
#include "rcsw/common/dbg.h"
#include "rcsw/utils/utils.h"

/*******************************************************************************
 * API Functions
 ******************************************************************************/
status_t simple_image_clamp_rgb(simple_image_t* const image) {
  RCSW_FPC_NV(ERROR, NULL != image);
  int const width = image->width;
  int const height = image->height;

  float* const __restrict__ red = image->red;
  float* const __restrict__ green = image->green;
  float* const __restrict__ blue = image->blue;

  float minv = 255.0;
  float maxv = 0.0;

#pragma omp parallel for schedule(static) reduction(min                   \
                                                    : minv) reduction(max \
                                                                      : maxv)
  for (int x = 0; x < width * height; ++x) {
    minv = RCSW_MIN(minv, red[x]);
    minv = RCSW_MIN(minv, green[x]);
    minv = RCSW_MIN(minv, blue[x]);

    maxv = RCSW_MAX(maxv, red[x]);
    maxv = RCSW_MAX(maxv, green[x]);
    maxv = RCSW_MAX(maxv, blue[x]);
  } /* for(x..) */

  if (minv >= 0. && maxv <= 255.0) {
    return OK;
  }

  float const scale = 255.0f / (maxv - minv);

#pragma omp parallel for schedule(static)
  for (int x = 0; x < width * height; ++x) {
    red[x] = (red[x] - minv) * scale;
    green[x] = (green[x] - minv) * scale;
    blue[x] = (blue[x] - minv) * scale;

    red[x] = utils_clamp_f255(red[x]);
    green[x] = utils_clamp_f255(green[x]);
    blue[x] = utils_clamp_f255(blue[x]);
  } /* for(x..) */
  return OK;
} /* simple_image_clamp_rgb() */

size_t simple_image_kernel2d_flops(simple_image_t const* const image,
                                   size_t kernel_dim) {
  size_t flops_per_pixel = kernel_dim * kernel_dim * 2;
  size_t n_pixels = 3 * image->width * image->height; /* 3 accounts for RGB */

  /* subtract the boundary since we skip it for simplicity */
  n_pixels -= image->width * 2;
  n_pixels -= image->height * 2;

  return flops_per_pixel * n_pixels;
} /* simple_image_kernel2d_flops() */

simple_image_t* simple_image_load(char const* const filename) {
  RCSW_FPC_NV(NULL, NULL != filename);
  int width;
  int height;
  int nchannels;
  uint8_t* simple_image = stbi_load(filename, &width, &height, &nchannels, 3);
  if (simple_image == NULL) {
    stbi_failure_reason();
    return NULL;
  }

  /* Now split simple_image into red/green/blue channels. */
  simple_image_t* im = simple_image_alloc(width, height);
  for (int x = 0; x < width * height; ++x) {
    im->red[x] = (float)simple_image[(x * nchannels) + 0];
    im->green[x] = (float)simple_image[(x * nchannels) + 1];
    im->blue[x] = (float)simple_image[(x * nchannels) + 2];
  }

  /* clean up the loaded data */
  stbi_image_free(simple_image);

  return im;
} /* simple_image_load() */

status_t simple_image_write_bmp(char const* const filename,
                                simple_image_t* const image) {
  RCSW_FPC_NV(ERROR, NULL != filename, NULL != image);
  RCSW_CHECK(OK == simple_image_clamp_rgb(image));

  /* First merge the red/green/blue channels. */
  size_t const im_size = image->width * image->height;
  uint8_t* image_bytes = malloc(im_size * 3 * sizeof(*image_bytes));
  for (size_t x = 0; x < im_size; ++x) {
    image_bytes[(x * 3) + 0] = (uint8_t)image->red[x];
    image_bytes[(x * 3) + 1] = (uint8_t)image->green[x];
    image_bytes[(x * 3) + 2] = (uint8_t)image->blue[x];
  } /* for(x..) */

  /* Now write to file. */
  int success =
      stbi_write_bmp(filename, image->width, image->height, 3, image_bytes);
  if (!success) {
    fprintf(stderr, "ERROR writing to '%s'\n", filename);
    stbi_failure_reason();
  }

  free(image_bytes);
  return OK;

error:
  return ERROR;
} /* simple_image_write_bmp() */

simple_image_t* simple_image_alloc(size_t width, size_t height) {
  simple_image_t* im = malloc(sizeof(*im));
  RCSW_CHECK_PTR(im);
  im->width = width;
  im->height = height;

  im->red = malloc(width * height * sizeof(*im->red));
  RCSW_CHECK_PTR(im->red);
  im->green = malloc(width * height * sizeof(*im->green));
  RCSW_CHECK_PTR(im->green);
  im->blue = malloc(width * height * sizeof(*im->blue));
  RCSW_CHECK_PTR(im->blue);
  return im;

error:
  simple_image_free(im);
  return NULL;
} /* simple_image_alloc() */

void simple_image_free(simple_image_t* const im) {
  if (im == NULL) {
    return;
  }
  if (im->red) {
    free(im->red);
  }
  if (im->blue) {
    free(im->blue);
  }
  if (im->green) {
    free(im->green);
  }
  free(im);
} /* simple_image_free() */
