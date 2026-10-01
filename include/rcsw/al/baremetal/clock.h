/**
 * \file
 *
 * \copyright 2023 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * \ingroup al
 *
 * \brief Clock interface on bare-metal targets.
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <time.h>

#include "rcsw/core/compilers.h"

/*******************************************************************************
 * Public API
 ******************************************************************************/
BEGIN_C_DECLS
#ifndef DOXYGEN_DOCUMENTATION_BUILD
/**
 * \brief Get the monotonic system time.
 *
 * \return The time.
 */
RCSW_API struct timespec clock_monotime(void);

/**
 * \brief Get the real system time.
 *
 * \return The time.
 */
RCSW_API struct timespec clock_realtime(void);

#endif
END_C_DECLS
