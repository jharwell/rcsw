/**
 * \file time_utils.c
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

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/utils/time_utils.h"
#include <time.h>

/*******************************************************************************
 * Forward Declarations
 ******************************************************************************/
BEGIN_C_DECLS

/*******************************************************************************
 * API Functions
 ******************************************************************************/
double time_monotonic_sec(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec + ts.tv_nsec * 1e-9;
} /* time_monotonic_sec() */

int time_ts_cmp(const struct timespec* const a, const struct timespec* const b) {
  if (a->tv_sec > b->tv_sec) {
    return 1;
  } else if (a->tv_sec < b->tv_sec) {
    return -1;
  } else if (a->tv_sec == b->tv_sec && a->tv_nsec > b->tv_nsec) {
    return 1;
  } else if (a->tv_sec == b->tv_sec && a->tv_nsec < b->tv_nsec) {
    return -1;
  }
  return 0;
} /* time_ts_cmp() */

void time_ts_add(struct timespec* const sum, const struct timespec* const val) {
  sum->tv_nsec += val->tv_nsec;
  sum->tv_sec += val->tv_sec + (sum->tv_nsec / ONEE9);
  sum->tv_nsec %= ONEE9;
} /* time_ts_add() */

void time_ts_diff(const struct timespec* const start,
                  const struct timespec* const end,
                  struct timespec* const diff) {
  if (end->tv_nsec - start->tv_nsec < 0) {
    diff->tv_sec = end->tv_sec - start->tv_sec - 1;
    diff->tv_nsec = ONEE9 + end->tv_nsec - start->tv_nsec;
  } else {
    diff->tv_sec = end->tv_sec - start->tv_sec;
    diff->tv_nsec = end->tv_nsec - start->tv_nsec;
  }
} /* time_ts_diff() */

status_t time_ts_ref_conv(const struct timespec* const in,
                          struct timespec* const out) {
  /* Get current time */
  RCSW_CHECK(0 == clock_gettime(CLOCK_REALTIME, out));

  /*
   * Convert from relative timeout to the abs timeout expected by
   * sem_wait().
   */
  out->tv_nsec += in->tv_nsec;
  out->tv_sec += in->tv_sec + (out->tv_nsec / ONEE9);
  out->tv_nsec %= ONEE9;
  return OK;

error:
  return ERROR;
} /* time_ts_ref_conv() */
END_C_DECLS
