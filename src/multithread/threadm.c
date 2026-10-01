/**
 * \file
 *
 * \copyright 2017 John Harwell
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#ifndef _GNU_SOURCE
/* This is required to get CPU_ZERO() and friends */
/* NOLINTNEXTLINE(bugprone-reserved-identifier,cert-dcl37-c) */
#define _GNU_SOURCE
#endif
#include "rcsw/multithread/threadm.h"

#include <errno.h>
#include <pthread.h>
#include <sched.h>

#include "rcsw/core/compilers.h"
#include "rcsw/core/core.h"

/*******************************************************************************
 * Public API
 ******************************************************************************/
BEGIN_C_DECLS

status_t threadm_core_lock(pthread_t thread, size_t core) {
  if (core >= CPU_SETSIZE) {
    errno = EINVAL; /* CPU_SET() would write past the end of the set */
    return ERROR;
  }
  cpu_set_t cpuset;

  CPU_ZERO(&cpuset);
  CPU_SET(core, &cpuset);
  int rc = pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
  if (0 != rc) {
    errno = rc; /* pthread functions return the error code */
    return ERROR;
  }
  return OK;
} /* threadm_core_lock() */

END_C_DECLS
