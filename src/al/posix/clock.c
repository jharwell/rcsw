/**
 * \file
 *
 * \copyright 2023 John Harwell
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/al/posix/clock.h"

#include <assert.h>

#include "rcsw/utils/time.h"

/*******************************************************************************
 * Public API
 ******************************************************************************/
BEGIN_C_DECLS

struct timespec clock_monotime(void) {
  struct timespec ts = {0};
  int             rc = clock_gettime(CLOCK_MONOTONIC, &ts);
  assert(0 == rc);
  return ts;
} /* clock_monotime() */

struct timespec clock_realtime(void) {
  struct timespec ts = {0};
  int             rc = clock_gettime(CLOCK_REALTIME, &ts);
  assert(0 == rc);
  return ts;
} /* clock_realtime() */

END_C_DECLS
