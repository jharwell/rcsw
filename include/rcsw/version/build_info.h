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
  /**
   * The current git revision.
   */
  const char*         git_rev;

  /**
   * "+" if the current tree is dirty, "" otherwise.
   */
  const char*         git_diff;

  /**
   * Current git tag if it exists, "" otherwise.
   */
  const char*         git_tag;

  /**
   * Current git branch if it exists.
   */
  const char*         git_branch;

  /**
   * Revelant compiler flags used to build the library/application.
   */
  const char*         compiler_flags;
  /* clang-format on */
};
