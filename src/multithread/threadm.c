/**
 * \file threadm.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "rcsw/multithread/threadm.h"

#include <pthread.h>
#include <sched.h>

#include "rcsw/rcsw.h"

/*******************************************************************************
 * Forward Declarations
 ******************************************************************************/
BEGIN_C_DECLS

/*******************************************************************************
 * Functions
 ******************************************************************************/
status_t threadm_core_lock(pthread_t thread, size_t core) {
  cpu_set_t cpuset;

  CPU_ZERO(&cpuset);
  CPU_SET(core, &cpuset);
  RCSW_CHECK(0 == pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset));
  return OK;

error:
  return ERROR;
} /* threadm_core_lock() */

END_C_DECLS
