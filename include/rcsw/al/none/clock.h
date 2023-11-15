/**
 * \file clock.h
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/rcsw.h"

/*******************************************************************************
 * API Functions
 ******************************************************************************/
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

/**
 * \brief Convert a relative timeout to an absolute one using \ref
 * clock_realtime() as a basis.
 *
 * Required for many POSIX functions.
 *
 * \param rel A relative timout.
 */
RCSW_API status_t clock_abs_timeout(const struct timespec * __restrict__ rel,
                                    struct timespec * out);
