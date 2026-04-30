/**
 * \file
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * \ingroup utils
 *
 * \brief Collection of \c struct timespec manipulation functions.
 *
 * \note These functions are POSIX-only and are not available in baremetal
 *       builds.
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stddef.h>
#include <stdint.h>
#include <time.h>

#include "rcsw/core/compilers.h"
#include "rcsw/core/core.h"

/*******************************************************************************
 * Public API
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
RCSW_API int utils_ts_cmp(const struct timespec* a,
                          const struct timespec* b) RCSW_PURE;

/**
 * \brief Add \p val to \p sum in place.
 *
 * \param sum Timespec accumulator (modified in place).
 * \param val Value to add.
 */
RCSW_API void utils_ts_add(struct timespec* __restrict__ sum,
                           const struct timespec* __restrict__ val);

/**
 * \brief Compute the difference between two timespecs: \p diff = \p end - \p
 * start.
 *
 * \param start The subtractee.
 * \param end   The subtractor.
 * \param diff  Output: result of end - start.
 *
 * \note If \p end < \p start, the result is undefined.
 */
RCSW_API void utils_ts_diff(const struct timespec* __restrict__ start,
                            const struct timespec* __restrict__ end,
                            struct timespec* __restrict__ diff);

/**
 * \brief Convert a timespec to a monotonic scalar in seconds.
 *
 * \param ts The timespec to convert.
 *
 * \return Monotonic time in seconds as a \c double.
 */
RCSW_API double utils_ts2mono(const struct timespec* const ts) RCSW_PURE;

/**
 * \brief Convert a timespec to a monotonic scalar in nanoseconds.
 *
 * \param ts The timespec to convert.
 *
 * \return Monotonic time in nanoseconds as a \c uint64_t.
 */
RCSW_API uint64_t utils_ts2monons(const struct timespec* const ts) RCSW_PURE;

/**
 * \brief Convert a monotonic nanosecond count to a timespec.
 *
 * \param val Nanosecond count.
 *
 * \return Equivalent \c struct timespec.
 */
RCSW_API struct timespec utils_monons2ts(uint64_t val) RCSW_CONST;

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
RCSW_API status_t utils_ts_make_abs(const struct timespec* __restrict__ rel,
                                    struct timespec* out);

/**
 * \brief Compute the remaining relative timeout from an absolute deadline.
 *
 * Subtracts the current wall-clock time from \p abs_deadline to produce the
 * remaining relative timeout in \p rel_out. If the deadline has already passed,
 * \p rel_out is set to {0, 0} and \ref ERROR is returned so callers can treat
 * an expired deadline the same as a timed-out wait.
 *
 * \param deadline An absolute \c CLOCK_REALTIME deadline, as produced by
 *                 \ref utils_ts_make_abs().
 *
 * \param out      Filled with the remaining relative timeout.
 *
 * \return \ref status_t. Returns \ref ERROR if the deadline has already passed.
 */
RCSW_API status_t utils_ts_make_rel(const struct timespec* deadline,
                                    struct timespec*       out);

END_C_DECLS
