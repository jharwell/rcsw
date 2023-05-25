/**
 * \file element.h
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/rcsw.h"

/*******************************************************************************
 * Class Definitions
 ******************************************************************************/
struct element8 {
  int32_t value1;
  int32_t value2;
};

struct element4 {
  int16_t value1;
  int16_t value2;
};

struct element2 {
  int8_t value1;
  int8_t value2;
};

struct element1 {
  int8_t value1;
};
