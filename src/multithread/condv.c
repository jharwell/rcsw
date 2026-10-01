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
#include "rcsw/multithread/condv.h"

#include <errno.h>

#include "rcsw/core/alloc.h"
#include "rcsw/core/flags.h"
#include "rcsw/core/fpc.h"
#include "rcsw/er/client.h"
#include "rcsw/utils/time.h"

/*******************************************************************************
 * Public API
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief pthread functions return an error code instead of setting errno;
 * copy it to errno so callers can tell, e.g., a timeout from a failure.
 */
static int rcsw_pthread_rc(int rc) {
  if (0 != rc) {
    errno = rc;
  }
  return rc;
}

struct condv* condv_init(struct condv* const cv_in, uint32_t flags) {
  struct condv* cv = rcsw_alloc(cv_in,
                                sizeof(struct condv),
                                flags & (RCSW_NOALLOC_HANDLE | RCSW_ZALLOC));
  if (NULL == cv) {
    errno = ENOMEM;
    return NULL;
  }
  cv->flags = flags;

  if (0 != rcsw_pthread_rc(pthread_cond_init(&cv->impl, NULL))) {
    /* never initialized: don't pthread_cond_destroy() it */
    rcsw_free(cv, flags & RCSW_NOALLOC_HANDLE);
    return NULL;
  }
  return cv;
}

void condv_destroy(struct condv* const cv) {
  RCSW_FPC_V(NULL != cv);

  pthread_cond_destroy(&cv->impl);
  rcsw_free(cv, cv->flags & RCSW_NOALLOC_HANDLE);
}

status_t condv_signal(struct condv* const cv) {
  RCSW_FPC_NV(ERROR, NULL != cv);
  RCSW_CHECK(0 == rcsw_pthread_rc(pthread_cond_signal(&cv->impl)));
  return OK;

error:
  return ERROR;
}

status_t condv_wait(struct condv* const cv, struct mutex* const mtx) {
  RCSW_FPC_NV(ERROR, NULL != cv, NULL != mtx);
  RCSW_CHECK(0 == rcsw_pthread_rc(pthread_cond_wait(&cv->impl, &mtx->impl)));
  return OK;

error:
  return ERROR;
}

status_t condv_timedwait(struct condv* const          cv,
                         struct mutex* const          mtx,
                         const struct timespec* const to) {
  RCSW_FPC_NV(ERROR, NULL != cv, NULL != mtx, NULL != to);
  struct timespec ts = {.tv_sec = 0, .tv_nsec = 0};

  RCSW_CHECK(OK == utils_ts_make_abs(to, &ts));
  return condv_timedwait_abs(cv, mtx, &ts);

error:
  return ERROR;
}

status_t condv_timedwait_abs(struct condv* const          cv,
                             struct mutex* const          mtx,
                             const struct timespec* const deadline) {
  RCSW_FPC_NV(ERROR, NULL != cv, NULL != mtx, NULL != deadline);
  RCSW_CHECK(0 == rcsw_pthread_rc(
                    pthread_cond_timedwait(&cv->impl, &mtx->impl, deadline)));
  return OK;

error:
  return ERROR;
}

status_t condv_broadcast(struct condv* const cv) {
  RCSW_FPC_NV(ERROR, NULL != cv);
  RCSW_CHECK(0 == rcsw_pthread_rc(pthread_cond_broadcast(&cv->impl)));
  return OK;

error:
  return ERROR;
}

END_C_DECLS
