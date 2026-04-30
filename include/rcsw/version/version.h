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
#include "rcsw/core/compilers.h"
#include "rcsw/version/meta_info.h"

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
/**
 * \brief The single provenance record for this RCSW build.
 *
 * Populated at compile time by the CMake template \c version.c.in. Valid as
 * soon as the translation unit that defines it has been loaded (i.e. from
 * program startup for static builds, or from shared-library load for dynamic
 * builds). Do not modify.
 *
 * Prefer the accessor functions below over direct field access so that call
 * sites remain insulated from future struct layout changes.
 */
extern struct meta_info rcsw_metadata;

/*******************************************************************************
 * Public API
 ******************************************************************************/
/**
 * \brief Return the release version string (e.g. \c "v1.2.3").
 */
RCSW_PURE static inline const char* rcsw_version(void) {
  return rcsw_metadata.version.version;
}

/**
 * \brief Return the abbreviated license notice.
 */
RCSW_PURE static inline const char* rcsw_license_abbrev(void) {
  return rcsw_metadata.version.license.abbrev;
}

/**
 * \brief Return the full license text.
 */
RCSW_PURE static inline const char* rcsw_license_full(void) {
  return rcsw_metadata.version.license.full;
}

/**
 * \brief Return the copyright notice string.
 */
RCSW_PURE static inline const char* rcsw_license_copyright(void) {
  return rcsw_metadata.version.license.copyright;
}

/**
 * \brief Return the short git commit hash at build time, or \c "" if
 *        unavailable.
 */
RCSW_PURE static inline const char* rcsw_build_git_rev(void) {
  return rcsw_metadata.build.git_rev;
}

/**
 * \brief Return \c "+" if the tree was dirty at build time, \c "" otherwise.
 */
RCSW_PURE static inline const char* rcsw_build_git_diff(void) {
  return rcsw_metadata.build.git_diff;
}

/**
 * \brief Return the git tag at build time, or \c "" if none.
 */
RCSW_PURE static inline const char* rcsw_build_git_tag(void) {
  return rcsw_metadata.build.git_tag;
}

/**
 * \brief Return the git branch at build time, or \c "" if unavailable.
 */
RCSW_PURE static inline const char* rcsw_build_git_branch(void) {
  return rcsw_metadata.build.git_branch;
}

/**
 * \brief Return the compiler flags used for this build.
 */
RCSW_PURE static inline const char* rcsw_build_compile_flags(void) {
  return rcsw_metadata.build.compile_flags;
}

/**
 * \brief Return the linker flags used for this build.
 */
RCSW_PURE static inline const char* rcsw_build_link_flags(void) {
  return rcsw_metadata.build.link_flags;
}

/**
 * \brief Return the calendar date of the build (\c __DATE__ format).
 */
RCSW_PURE static inline const char* rcsw_build_date(void) {
  return rcsw_metadata.build.date;
}

/**
 * \brief Return the wall-clock time of the build (\c __TIME__ format).
 */
RCSW_PURE static inline const char* rcsw_build_time(void) {
  return rcsw_metadata.build.time;
}
