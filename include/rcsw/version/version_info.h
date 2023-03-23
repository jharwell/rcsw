/**
 * \file version_info.h
 *
 * \copyright 2022 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/version/license_info.h"

/*******************************************************************************
 * Struct Definitions
 ******************************************************************************/
/**
 * \struct version_info
 * \ingroup version
 *
 * \brief Version information (duh).
 */
struct version_info {
  /* clang-format off */
  const char*         version;
  struct license_info license;
  /* clang-format on */
};
