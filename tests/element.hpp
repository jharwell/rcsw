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
bool operator==(const element8 &rhs) const {
  return this->value1 == rhs.value1 &&
      this->value2 == rhs.value2;
}

bool operator==(const element4 &rhs) const {
  return this->value1 == rhs.value1 &&
      this->value2 == rhs.value2;
}

bool operator==(const element2 &rhs) const {
  return this->value1 == rhs.value1 &&
      this->value2 == rhs.value2;
}

bool operator==(const element1 &rhs) const {
  return this->value1 == rhs.value1;
}
