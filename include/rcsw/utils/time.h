/**
 * \file time.h
 * \ingroup utils
 * \brief Collection of timespec/time manipulation functions.
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/rcsw.h"

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Compare two timespecs
 *
 * \param a Timespec #1
 * \param b Timespec #2
 *
 * \return <, =, or > 0, dedpending if a is found to be >, =, or > b
 */
RCSW_API int time_ts_cmp(const struct timespec * a,
                         const struct timespec * b) RCSW_PURE;

/**
 * \brief Add two timespecs in place
 *
 * \param sum Timespec #1 (also where the result is put)
 * \param val Timespec #2
 *
 */
RCSW_API void time_ts_add(struct timespec * __restrict__ sum,
                          const struct timespec * __restrict__ val);

/**
 * \brief Get the difference between two timespecs
 *
 * diff = end - start
 *
 * \param start The subtractee
 * \param end The subtractor
 * \param diff The result
 *
 */
RCSW_API void time_ts_diff(const struct timespec * __restrict__ start,
                           const struct timespec * __restrict__ end,
                           struct timespec * __restrict__ diff);

/**
 * \brief Convert a timespec to a scalar monotonic count in seconds.
 */
RCSW_API double time_ts2mono(const struct timespec* const ts) RCSW_PURE;

/**
 * \brief Convert a timespec to a scalar monotonic count in nanoseconds.
 */
RCSW_API size_t time_ts2monons(const struct timespec* const ts) RCSW_PURE;

/**
 * \brief Convert a scalar monotonic count in nanoseconds to a timespec.
 */
RCSW_API struct timespec time_monons2ts(size_t val);

END_C_DECLS
