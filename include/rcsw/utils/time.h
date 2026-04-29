/**
 * \file time.h
 * \ingroup utils
 * \brief Collection of \c struct timespec manipulation functions.
 *
 * \note These functions are POSIX-only and are not available in baremetal
 *       builds (\c RCSW_BUILD_FOR=BAREMETAL).
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
 * \return <0 if a < b, 0 if a == b, >0 if a > b.
 */
RCSW_API int time_ts_cmp(const struct timespec * a,
                         const struct timespec * b) RCSW_PURE;

/**
 * \brief Add \p val to \p sum in place.
 *
 * \param sum Timespec accumulator (modified in place).
 * \param val Value to add.
 */
RCSW_API void time_ts_add(struct timespec * __restrict__ sum,
                          const struct timespec * __restrict__ val);

/**
 * \brief Compute the difference between two timespecs: \p diff = \p end - \p start.
 *
 * \param start The subtractee.
 * \param end   The subtractor.
 * \param diff  Output: result of end - start.
 */
RCSW_API void time_ts_diff(const struct timespec * __restrict__ start,
                           const struct timespec * __restrict__ end,
                           struct timespec * __restrict__ diff);

/**
 * \brief Convert a timespec to a monotonic scalar in seconds.
 *
 * \param ts The timespec to convert.
 *
 * \return Monotonic time in seconds as a \c double.
 */
RCSW_API double time_ts2mono(const struct timespec* const ts) RCSW_PURE;

/**
 * \brief Convert a timespec to a monotonic scalar in nanoseconds.
 *
 * \param ts The timespec to convert.
 *
 * \return Monotonic time in nanoseconds as a \c size_t.
 */
RCSW_API size_t time_ts2monons(const struct timespec* const ts) RCSW_PURE;

/**
 * \brief Convert a monotonic nanosecond count to a timespec.
 *
 * \param val Nanosecond count.
 *
 * \return Equivalent \c struct timespec.
 */
RCSW_API struct timespec time_monons2ts(size_t val) RCSW_CONST;

/**
 * \brief Convert a relative timeout to an absolute one.
 *
 * Uses \ref clock_realtime() as the basis. Required by many POSIX blocking
 * functions (e.g., \c pthread_cond_timedwait()).
 *
 * \param rel Relative timeout.
 * \param out Output: absolute timeout equivalent to now + \p rel.
 *
 * \return \ref status_t
 */
RCSW_API status_t time_ts_make_abs(const struct timespec * __restrict__ rel,
                                   struct timespec * out);

END_C_DECLS
