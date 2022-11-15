/**
 * \file threadm.c
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
#define _GNU_SOURCE
#include "rcsw/multithread/threadm.h"
#include <pthread.h>
#include <sched.h>
#include "rcsw/common/common.h"

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
