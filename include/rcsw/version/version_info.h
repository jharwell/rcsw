/**
 * \file
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
 * \brief The release version and associated license notice for a project.
 *
 * The \c version string is in whatever format the project chooses (semver,
 * date-based, etc.). The \c license fields hold the pre-formatted text
 * produced by the \ref RCSW_LICENSE_SHORT and \ref RCSW_LICENSE_FULL macros,
 * suitable for printing at program startup.
 */
struct version_info {
  /* clang-format off */

  /**
   * Human-readable version string (e.g. \c "v1.2.3").
   */
  const char*         version;

  /**
   * License notice for the project.
   */
  struct license_info license;
  /* clang-format on */
};
