/**
 * \file mt_cond.c
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
#include "rcsw/multithread/mt_cond.h"
#include "rcsw/common/dbg.h"
#include "rcsw/common/fpc.h"
#include "rcsw/utils/time_utils.h"

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

mt_cond_t* mt_cond_init(mt_cond_t* const cv_in, uint32_t flags) {
  mt_cond_t* cv = NULL;
  if (flags & MT_APP_DOMAIN_MEM) {
    cv = cv_in;
  } else {
    cv = calloc(1, sizeof(mt_cond_t));
  }
  RCSW_CHECK_PTR(cv);
  cv->flags = flags;

  RCSW_CHECK(0 == pthread_cond_init(&cv->cv, NULL));
  return cv;

error:
  mt_cond_destroy(cv);
  return NULL;
} /* mt_cond_init() */

void mt_cond_destroy(mt_cond_t* const cv) {
  RCSW_FPC_V(NULL != cv);

  pthread_cond_destroy(&cv->cv);
  if (cv->flags & MT_APP_DOMAIN_MEM) {
    free(cv);
  }
} /* mt_cond_destroy() */

status_t mt_cond_signal(mt_cond_t* const cv) {
  RCSW_FPC_NV(ERROR, NULL != cv);
  RCSW_CHECK(0 == pthread_cond_signal(&cv->cv));
  return OK;

error:
  return ERROR;
} /* mt_cond_signal() */

status_t mt_cond_wait(mt_cond_t* const cv, mt_mutex_t* const mutex) {
  RCSW_FPC_NV(ERROR, NULL != cv);
  RCSW_CHECK(0 == pthread_cond_wait(&cv->cv, &mutex->mutex));
  return OK;

error:
  return ERROR;
} /* mt_cond_wait() */

status_t mt_cond_timedwait(mt_cond_t* const cv,
                           mt_mutex_t* const mutex,
                           const struct timespec* const to) {
  RCSW_FPC_NV(ERROR, NULL != cv, NULL != to);
  struct timespec ts = {.tv_sec = 0, .tv_nsec = 0};

  /* Get current time */
  RCSW_CHECK(OK == time_ts_ref_conv(to, &ts));
  RCSW_CHECK(0 == pthread_cond_timedwait(&cv->cv, &mutex->mutex, &ts));
  return OK;

error:
  return ERROR;
} /* mt_cond_timedwait() */

status_t mt_cond_broadcast(mt_cond_t* const cv) {
  RCSW_FPC_NV(ERROR, NULL != cv);
  RCSW_CHECK(0 == pthread_cond_broadcast(&cv->cv));
  return OK;

error:
  return ERROR;
} /* mt_cond_broadcast() */

END_C_DECLS
