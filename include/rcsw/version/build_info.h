/**
 * \file
 *
 * \copyright 2022 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Struct Definitions
 ******************************************************************************/
/**
 * \struct build_info
 * \ingroup version
 *
 * \brief Snapshot of the build environment captured at compile time.
 *
 * Populated automatically by the CMake template \c version.c.in. Fields
 * sourced from the LIBRA build framework are set to \c "" when not available
 * (e.g. when building outside a git repository).
 */
struct build_info {
  /**
   * The git commit hash (short form) at the time of the build, or \c ""
   * if unavailable.
   */
  const char* const git_rev;

  /**
   * \c "+" if the working tree had uncommitted changes at build time,
   * \c "" otherwise.
   */
  const char* const git_diff;

  /**
   * The git tag pointing at the build commit, or \c "" if none exists.
   */
  const char* const git_tag;

  /**
   * The git branch active at build time, or \c "" if unavailable.
   */
  const char* const git_branch;

  /**
   * Relevant compiler flags used to build the library or application.
   */
  const char* const compile_flags;

  /**
   * Relevant linker flags used to build the library or application.
   */
  const char* const link_flags;

  /**
   * The calendar date of the build, as reported by \c __DATE__.
   */
  const char* const date;

  /**
   * The wall-clock time of the build, as reported by \c __TIME__.
   */
  const char* const time;
};
