/**
 * \file time_utils.h
 * \ingroup utils
 * \brief Collection of timespec/time manipulation functions.
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * This file is part of RCSW.
 *
 * RCSW is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * RCSW is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * RCSW.  If not, see <http://www.gnu.org/licenses/
 */

#ifndef INCLUDE_RCSW_UTILS_TIME_UTILS_H_
#define INCLUDE_RCSW_UTILS_TIME_UTILS_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/common/common.h"

/*******************************************************************************
 * Function Prototypes
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
int time_ts_cmp(const struct timespec * a,
                const struct timespec * b) RCSW_PURE;

/**
 * \brief Add two timespecs in place
 *
 * \param sum Timespec #1 (also where the result is put)
 * \param val Timespec #2
 *
 */
void time_ts_add(struct timespec * __restrict__ sum,
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
void time_ts_diff(const struct timespec * __restrict__ start,
                  const struct timespec * __restrict__ end,
                  struct timespec * __restrict__ diff);

status_t time_ts_ref_conv(const struct timespec * __restrict__ in,
                          struct timespec * __restrict__ out);

/**
 * \brief Get the monotonic system time
 *
 * \return The time
 */
double time_monotonic_sec(void);

END_C_DECLS

#endif /* INCLUDE_RCSW_UTILS_TIME_UTILS_H_ */
