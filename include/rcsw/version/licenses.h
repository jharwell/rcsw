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
#include "rcsw/core/core.h"

/*******************************************************************************
 * License String Constants
 ******************************************************************************/
/**
 * \brief Full GPLv3+ license notice, suitable for printing at program start.
 */
RCSW_API extern const char rcsw_license_full_gplv3_text[];

/**
 * \brief Short GPLv3+ license notice.
 */
RCSW_API extern const char rcsw_license_short_gplv3_text[];

/**
 * \brief Full LGPLv3+ license notice, suitable for printing at program start.
 */
RCSW_API extern const char rcsw_license_full_lgplv3_text[];

/**
 * \brief Short LGPLv3+ license notice.
 */
RCSW_API extern const char rcsw_license_short_lgplv3_text[];

/**
 * \brief Full MIT license notice, suitable for printing at program start.
 */
RCSW_API extern const char rcsw_license_full_mit_text[];

/**
 * \brief Short MIT license notice.
 */
RCSW_API extern const char rcsw_license_short_mit_text[];

/*******************************************************************************
 * Macros
 ******************************************************************************/
/**
 * \def RCSW_COPYRIGHT(year, author)
 *
 * Produce a copyright notice string for embedding in \ref license_info.
 *
 * \param year   The copyright year as an integer token (e.g. \c 2023).
 * \param author The copyright holder as a quoted string literal
 *               (e.g. \c "John Harwell"). Must be a string literal, not a
 *               bare token sequence, so that multi-word names are handled
 *               correctly.
 */
#define RCSW_COPYRIGHT(year, author) \
  "Copyright (c) " RCSW_XSTR(year) " " author ".\n"

/**
 * \def RCSW_LICENSE_SHORT(license)
 *
 * \brief Select the short-form license notice for \a license.
 *
 * Expands to the corresponding \c rcsw_license_short_<license>_text pointer,
 * suitable for assigning to the \c abbrev field of \ref license_info.
 *
 * \param license License identifier token. Must be one of: \c GPLV3,
 *                \c LGPLV3, \c MIT.
 */
#define RCSW_LICENSE_SHORT(license) \
  RCSW_JOIN3(rcsw_license_short_, license, _text)

/**
 * \def RCSW_LICENSE_FULL(license)
 *
 * \brief Select the full license notice for \a license.
 *
 * Expands to the corresponding \c rcsw_license_full_<license>_text pointer,
 * suitable for assigning to the \c full field of \ref license_info.
 *
 * \param license License identifier token. Must be one of: \c gplv3,
 *                \c lgplv3, \c mit.
 */
#define RCSW_LICENSE_FULL(license) RCSW_JOIN3(rcsw_license_full_, license, _text)
