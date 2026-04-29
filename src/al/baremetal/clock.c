/**
 * \file
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/al/baremetal/clock.h"

/*******************************************************************************
 * Public API
 ******************************************************************************/
BEGIN_C_DECLS

struct timespec clock_monotime(void) {
  struct timespec ts = {.tv_sec = 0, .tv_nsec = 0};
  return ts;
}

struct timespec clock_realtime(void) {
  struct timespec ts = {.tv_sec = 0, .tv_nsec = 0};
  return ts;
}

END_C_DECLS
