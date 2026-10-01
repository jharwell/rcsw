/**
 * \file
 *
 * \copyright 2026 John Harwell
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/version/version.h"

/*******************************************************************************
 * Public API
 ******************************************************************************/
const char* rcsw_version(void) { return g_rcsw_metadata.version.version; }

const char* rcsw_license_abbrev(void) {
  return g_rcsw_metadata.version.license.abbrev;
}

const char* rcsw_license_full(void) {
  return g_rcsw_metadata.version.license.full;
}

const char* rcsw_license_copyright(void) {
  return g_rcsw_metadata.version.license.copyright;
}

const char* rcsw_build_git_rev(void) { return g_rcsw_metadata.build.git_rev; }

const char* rcsw_build_git_diff(void) { return g_rcsw_metadata.build.git_diff; }

const char* rcsw_build_git_tag(void) { return g_rcsw_metadata.build.git_tag; }

const char* rcsw_build_git_branch(void) {
  return g_rcsw_metadata.build.git_branch;
}

const char* rcsw_build_compile_flags(void) {
  return g_rcsw_metadata.build.compile_flags;
}

const char* rcsw_build_link_flags(void) {
  return g_rcsw_metadata.build.link_flags;
}

const char* rcsw_build_date(void) { return g_rcsw_metadata.build.date; }

const char* rcsw_build_time(void) { return g_rcsw_metadata.build.time; }
