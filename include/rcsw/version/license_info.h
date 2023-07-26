/**
 * \file license_info.h
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

/*******************************************************************************
 * Struct Definitions
 ******************************************************************************/
/**
 * \struct license_info
 * \ingroup version
 *
 * \brief License information for a project.
 *
 * Useful to show on application/library load.
 */
struct license_info {
  /**
   * Abbreviated license text.
   */
  const char* abbrev;

  /**
   * Full license text.
   */
  const char* full;

  /**
   * Copyright text.
   */
  const char* copyright;
};
