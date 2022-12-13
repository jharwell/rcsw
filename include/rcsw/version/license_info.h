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
 * \brief License information (duh).
 */
struct license_info {
  const char* abbrev;
  const char* full;
  const char* copyright;
};
