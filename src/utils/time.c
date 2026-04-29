/**
 * \file
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/utils/time.h"

#include "rcsw/al/clock.h"
#include "rcsw/core/core.h"
#include "rcsw/core/fpc.h"

/*******************************************************************************
 * Public API
 ******************************************************************************/
BEGIN_C_DECLS

double utils_ts2mono(const struct timespec* const ts) {
  return (double)ts->tv_sec + (double)ts->tv_nsec * 1.0e-9;
}

uint64_t utils_ts2monons(const struct timespec* const ts) {
  return (uint64_t)ts->tv_sec * RCSW_E9 + (uint64_t)ts->tv_nsec;
}

struct timespec utils_monons2ts(uint64_t val) {
  struct timespec ts;
  ts.tv_sec  = (long int)(val / RCSW_E9);
  ts.tv_nsec = (long int)(val % RCSW_E9);
  return ts;
}

int utils_ts_cmp(const struct timespec* const a, const struct timespec* const b) {
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
}

void utils_ts_add(struct timespec* const sum, const struct timespec* const val) {
  sum->tv_nsec += val->tv_nsec;
  sum->tv_sec += val->tv_sec + (sum->tv_nsec / RCSW_E9);
  sum->tv_nsec %= RCSW_E9;
}

void utils_ts_diff(const struct timespec* const start,
                   const struct timespec* const end,
                   struct timespec* const       diff) {
  if (end->tv_nsec - start->tv_nsec < 0) {
    diff->tv_sec  = end->tv_sec - start->tv_sec - 1;
    diff->tv_nsec = RCSW_E9 + end->tv_nsec - start->tv_nsec;
  } else {
    diff->tv_sec  = end->tv_sec - start->tv_sec;
    diff->tv_nsec = end->tv_nsec - start->tv_nsec;
  }
}

status_t utils_ts_make_abs(const struct timespec* const rel,
                           struct timespec* const       out) {
  /* Get current time */
  *out = clock_realtime();

  utils_ts_add(out, rel);

  return OK;
}

status_t utils_ts_make_rel(const struct timespec* const deadline,
                           struct timespec* const       out) {
  RCSW_FPC_NV(ERROR, NULL != deadline, NULL != out);

  struct timespec now;
  RCSW_CHECK(0 == clock_gettime(CLOCK_REALTIME, &now));

  out->tv_sec  = deadline->tv_sec - now.tv_sec;
  out->tv_nsec = deadline->tv_nsec - now.tv_nsec;

  if (out->tv_nsec < 0) {
    out->tv_sec -= 1;
    out->tv_nsec += RCSW_E9;
  }

  /* deadline already passed */
  if (out->tv_sec < 0) {
    out->tv_sec  = 0;
    out->tv_nsec = 0;
    return ERROR;
  }

  return OK;

error:
  return ERROR;
}

END_C_DECLS
