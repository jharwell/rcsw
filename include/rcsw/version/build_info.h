/**
 * \file build_info.h
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
 * \struct build_info
 * \ingroup version
 *
 * \brief Build information (duh).
 */
struct build_info {
  /* clang-format off */
  const char*         git_rev;
  const char*         git_diff;
  const char*         git_tag;
  const char*         git_branch;
  const char*         compiler_flags;
  /* clang-format on */
};
