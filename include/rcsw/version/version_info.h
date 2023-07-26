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

  /**
   * The version for the project (in whatever format).
   */
  const char*         version;

  /**
   * License info for the project.
   */
  struct license_info license;
  /* clang-format on */
};
