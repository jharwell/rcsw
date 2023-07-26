/**
 * \file meta_info.h
 *
 * \copyright 2022 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/rcsw.h"
#include "rcsw/version/version_info.h"
#include "rcsw/version/build_info.h"

/*******************************************************************************
 * Struct Definitions
 ******************************************************************************/
/**
 * \struct meta_info
 * \ingroup version
 *
 * \brief Metadata information about the project, defined as version+build info.
 */
struct meta_info {
  /* clang-format off */
  /**
   * Version information for the project.
   */
  struct version_info version;

  /**
   * Build information for the project.
   */
  struct build_info   build;
  /* clang-format on */
};
