/**
 * \file time.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/utils/time.h"

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
  return time_ts2mono(&ts);
} /* time_monotonic_sec() */

double time_ts2mono(const struct timespec* const ts) {
  return ts->tv_sec + ts->tv_nsec * 1.0 / ONEE9;
} /* time_ts2mono() */

size_t time_ts2monons(const struct timespec* const ts) {
  return ts->tv_sec * ONEE9 + ts->tv_nsec;
} /* time_ts2mono() */

struct timespec time_monons2ts(size_t val) {
  struct timespec ts;
  ts.tv_sec = val / ONEE9;
  ts.tv_nsec = val % ONEE9;
  return ts;
} /* time_monons2ts() */

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

status_t time_ts_make_abs(const struct timespec* const in,
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
