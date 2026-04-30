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
#include "rcsw/multithread/condv.h"

#include "rcsw/core/alloc.h"
#include "rcsw/core/fpc.h"
#include "rcsw/er/client.h"
#include "rcsw/utils/time.h"

/*******************************************************************************
 * Public API
 ******************************************************************************/
BEGIN_C_DECLS

struct condv* condv_init(struct condv* const cv_in, uint32_t flags) {
  struct condv* cv =
    rcsw_alloc(cv_in, sizeof(struct condv), flags & RCSW_NOALLOC_HANDLE);
  RCSW_CHECK_PTR(cv);
  cv->flags = flags;

  RCSW_CHECK(0 == pthread_cond_init(&cv->impl, NULL));
  return cv;

error:
  condv_destroy(cv);
  return NULL;
}

void condv_destroy(struct condv* const cv) {
  RCSW_FPC_V(NULL != cv);

  pthread_cond_destroy(&cv->impl);
  rcsw_free(cv, cv->flags & RCSW_NOALLOC_HANDLE);
}

status_t condv_signal(struct condv* const cv) {
  RCSW_FPC_NV(ERROR, NULL != cv);
  RCSW_CHECK(0 == pthread_cond_signal(&cv->impl));
  return OK;

error:
  return ERROR;
}

status_t condv_wait(struct condv* const cv, struct mutex* const mtx) {
  RCSW_FPC_NV(ERROR, NULL != cv);
  RCSW_CHECK(0 == pthread_cond_wait(&cv->impl, &mtx->impl));
  return OK;

error:
  return ERROR;
}

status_t condv_timedwait(struct condv* const          cv,
                         struct mutex* const          mtx,
                         const struct timespec* const to) {
  RCSW_FPC_NV(ERROR, NULL != cv, NULL != mtx, NULL != to);
  struct timespec ts = {.tv_sec = 0, .tv_nsec = 0};

  /* Get current time */
  RCSW_CHECK(OK == utils_ts_make_abs(to, &ts));
  RCSW_CHECK(0 == pthread_cond_timedwait(&cv->impl, &mtx->impl, &ts));
  return OK;

error:
  return ERROR;
}

status_t condv_broadcast(struct condv* const cv) {
  RCSW_FPC_NV(ERROR, NULL != cv);
  RCSW_CHECK(0 == pthread_cond_broadcast(&cv->impl));
  return OK;

error:
  return ERROR;
}

END_C_DECLS
