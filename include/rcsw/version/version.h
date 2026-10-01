/**
 * \file
 *
 * \copyright 2022 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * \ingroup version
 *
 * \brief Accessors for RCSW's version, license, and build information.
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
extern const struct meta_info g_rcsw_metadata;

/*******************************************************************************
 * Public API
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Return the release version string (e.g. \c "v1.2.3").
 */
RCSW_API RCSW_CONST const char* rcsw_version(void);

/**
 * \brief Return the abbreviated license notice.
 */
RCSW_API RCSW_CONST const char* rcsw_license_abbrev(void);

/**
 * \brief Return the full license text.
 */
RCSW_API RCSW_CONST const char* rcsw_license_full(void);

/**
 * \brief Return the copyright notice string.
 */
RCSW_API RCSW_CONST const char* rcsw_license_copyright(void);

/**
 * \brief Return the short git commit hash at build time, or \c "" if
 *        unavailable.
 */
RCSW_API RCSW_CONST const char* rcsw_build_git_rev(void);

/**
 * \brief Return \c "+" if the tree was dirty at build time, \c "" otherwise.
 */
RCSW_API RCSW_CONST const char* rcsw_build_git_diff(void);

/**
 * \brief Return the git tag at build time, or \c "" if none.
 */
RCSW_API RCSW_CONST const char* rcsw_build_git_tag(void);

/**
 * \brief Return the git branch at build time, or \c "" if unavailable.
 */
RCSW_API RCSW_CONST const char* rcsw_build_git_branch(void);

/**
 * \brief Return the compiler flags used for this build.
 */
RCSW_API RCSW_CONST const char* rcsw_build_compile_flags(void);

/**
 * \brief Return the linker flags used for this build.
 */
RCSW_API RCSW_CONST const char* rcsw_build_link_flags(void);

/**
 * \brief Return the calendar date of the build (\c __DATE__ format).
 */
RCSW_API RCSW_CONST const char* rcsw_build_date(void);

/**
 * \brief Return the wall-clock time of the build (\c __TIME__ format).
 */
RCSW_API RCSW_CONST const char* rcsw_build_time(void);

END_C_DECLS
