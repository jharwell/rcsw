/**
 * \file time.c
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/al/posix/clock.h"

#include <time.h>

#include "rcsw/utils/time.h"

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

struct timespec clock_monotime(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts;
} /* clock_monotime() */

struct timespec clock_realtime(void) {
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  return ts;
} /* clock_realtime() */

END_C_DECLS
