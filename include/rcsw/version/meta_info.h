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
#include "rcsw/version/build_info.h"
#include "rcsw/version/version_info.h"

/*******************************************************************************
 * Struct Definitions
 ******************************************************************************/
/**
 * \struct meta_info
 * \ingroup version
 *
 * \brief Complete provenance record for an RCSW-based library or application.
 *
 * Combines \ref version_info (release version + license) with \ref build_info
 * (git state + compiler flags + timestamps). The single global instance of this
 * struct is \ref rcsw_metadata, which is populated at compile time by the CMake
 * template \c version.c.in. Read it through the accessor functions declared in
 * \c version.h rather than accessing fields directly, so that your code remains
 * insulated from future layout changes.
 */
struct meta_info {
  /* clang-format off */
  /**
   * Release version and license notice for the project.
   */
  struct version_info version;

  /**
   * Build environment snapshot captured at compile time.
   */
  struct build_info   build;
  /* clang-format on */
};
