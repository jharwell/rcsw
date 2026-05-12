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
#include "tests/element.h"

/*******************************************************************************
 * Class Definitions
 ******************************************************************************/
static inline auto operator==(const element8& lhs, const element8 &rhs) -> bool {
  return lhs.value1 == rhs.value1 && lhs.value2 == rhs.value2;
}

static inline auto operator==(const element4 &lhs, const element4 &rhs) -> bool {
  return lhs.value1 == rhs.value1 && lhs.value2 == rhs.value2;
}

static inline auto operator==(const element2 &lhs, const element2 &rhs) -> bool {
  return lhs.value1 == rhs.value1 && lhs.value2 == rhs.value2;
}

static inline auto operator==(const element1 &lhs, const element1 &rhs) -> bool {
  return lhs.value1 == rhs.value1;
}
