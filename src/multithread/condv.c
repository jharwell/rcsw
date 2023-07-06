/**
 * \file condv.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/multithread/condv.h"

#include "rcsw/er/client.h"
#include "rcsw/common/fpc.h"
#include "rcsw/utils/time.h"

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

struct condv* condv_init(struct condv* const cv_in, uint32_t flags) {
  struct condv* cv = NULL;
  if (flags & RCSW_NOALLOC_HANDLE) {
    cv = cv_in;
  } else {
    cv = calloc(1, sizeof(struct condv));
  }
  RCSW_CHECK_PTR(cv);
  cv->flags = flags;

  RCSW_CHECK(0 == pthread_cond_init(&cv->impl, NULL));
  return cv;

error:
  condv_destroy(cv);
  return NULL;
} /* condv_init() */

void condv_destroy(struct condv* const cv) {
  RCSW_FPC_V(NULL != cv);

  pthread_cond_destroy(&cv->impl);
  if (cv->flags & RCSW_NOALLOC_HANDLE) {
    free(cv);
  }
} /* condv_destroy() */

status_t condv_signal(struct condv* const cv) {
  RCSW_FPC_NV(ERROR, NULL != cv);
  RCSW_CHECK(0 == pthread_cond_signal(&cv->impl));
  return OK;

error:
  return ERROR;
} /* condv_signal() */

status_t condv_wait(struct condv* const cv, struct mutex* const mtx) {
  RCSW_FPC_NV(ERROR, NULL != cv);
  RCSW_CHECK(0 == pthread_cond_wait(&cv->impl, &mtx->impl));
  return OK;

error:
  return ERROR;
} /* condv_wait() */

status_t condv_timedwait(struct condv* const cv,
                         struct mutex* const mtx,
                         const struct timespec* const to) {
  RCSW_FPC_NV(ERROR, NULL != cv, NULL != mtx, NULL != to);
  struct timespec ts = { .tv_sec = 0, .tv_nsec = 0 };

  /* Get current time */
  RCSW_CHECK(OK == time_ts_ref_conv(to, &ts));
  RCSW_CHECK(0 == pthread_cond_timedwait(&cv->impl, &mtx->impl, &ts));
  return OK;

error:
  return ERROR;
} /* struct condvimedwait() */

status_t condv_broadcast(struct condv* const cv) {
  RCSW_FPC_NV(ERROR, NULL != cv);
  RCSW_CHECK(0 == pthread_cond_broadcast(&cv->impl));
  return OK;

error:
  return ERROR;
} /* condv_broadcast() */

END_C_DECLS
