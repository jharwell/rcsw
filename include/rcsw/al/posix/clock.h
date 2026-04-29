/**
 * \file
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
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

END_C_DECLS
