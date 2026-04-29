/**
 * \file
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * \ingroup utils
 *
 * \brief Endianness checking macros/functions.
 */

#pragma once

/******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/al/types.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/
/**
 * \brief Test for little-endian architecture at runtime.
 */
static inline bool_t utils_is_little_endian(void) {
  union {
    uint16_t u;
    uint8_t  c[2];
  } x = {.u = 1};
  return x.c[0] == 1;
}

/**
 * \brief Test for big-endian architecture at runtime.
 */
static inline bool_t utils_is_big_endian(void) {
  return !utils_is_little_endian();
}
