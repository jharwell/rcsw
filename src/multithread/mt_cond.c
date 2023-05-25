/**
 * \file mt_cond.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/multithread/mt_cond.h"
#include "rcsw/common/dbg.h"
#include "rcsw/common/fpc.h"
#include "rcsw/utils/time.h"

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
