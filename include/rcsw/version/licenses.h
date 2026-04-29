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
RCSW_API extern const char RCSW_LICENSE_FULL_GPLV3_TEXT[];

/**
 * \brief Short GPLv3+ license notice.
 */
RCSW_API extern const char RCSW_LICENSE_SHORT_GPLV3_TEXT[];

/**
 * \brief Full LGPLv3+ license notice, suitable for printing at program start.
 */
RCSW_API extern const char RCSW_LICENSE_FULL_LGPLV3_TEXT[];

/**
 * \brief Short LGPLv3+ license notice.
 */
RCSW_API extern const char RCSW_LICENSE_SHORT_LGPLV3_TEXT[];

/**
 * \brief Full MIT license notice, suitable for printing at program start.
 */
RCSW_API extern const char RCSW_LICENSE_FULL_MIT_TEXT[];

/**
 * \brief Short MIT license notice.
 */
RCSW_API extern const char RCSW_LICENSE_SHORT_MIT_TEXT[];

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
 * Expands to the corresponding \c RCSW_LICENSE_SHORT_<license>_TEXT pointer,
 * suitable for assigning to the \c abbrev field of \ref license_info.
 *
 * \param license License identifier token. Must be one of: \c GPLV3,
 *                \c LGPLV3, \c MIT.
 */
#define RCSW_LICENSE_SHORT(license) \
  RCSW_JOIN3(RCSW_LICENSE_SHORT_, license, _TEXT)

/**
 * \def RCSW_LICENSE_FULL(license)
 *
 * \brief Select the full license notice for \a license.
 *
 * Expands to the corresponding \c RCSW_LICENSE_FULL_<license>_TEXT pointer,
 * suitable for assigning to the \c full field of \ref license_info.
 *
 * \param license License identifier token. Must be one of: \c GPLV3,
 *                \c LGPLV3, \c MIT.
 */
#define RCSW_LICENSE_FULL(license) RCSW_JOIN3(RCSW_LICENSE_FULL_, license, _TEXT)
