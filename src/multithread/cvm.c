/**
 * \file mt_cvm.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/multithread/cvm.h"

#include "rcsw/common/fpc.h"
#include "rcsw/er/client.h"
#include "rcsw/utils/time.h"

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

struct cvm* cvm_init(struct cvm* const cvm_in, uint32_t flags) {
  struct cvm* cvm = NULL;
  if (flags & RCSW_NOALLOC_HANDLE) {
    cvm = cvm_in;
  } else {
    cvm = calloc(1, sizeof(struct cvm));
  }
  RCSW_CHECK_PTR(cvm);
  cvm->flags = flags;

  RCSW_CHECK(OK == condv_init(&cvm->cv, cvm->flags));
  RCSW_CHECK(OK == mutex_init(&cvm->mtx, cvm->flags));
  return cvm;

error:
  mt_cvm_destroy(cvm);
  return NULL;
} /* mt_cvm_init() */

void mt_cvm_destroy(struct cvm* const cvm) {
  RCSW_FPC_V(NULL != cvm);

  condv_destroy(&cvm->cv);
  mutex_destroy(&cvm->mtx);
  if (cvm->flags & RCSW_NOALLOC_HANDLE) {
    free(cvm);
  }
} /* mt_cvm_destroy() */
status_t cvm_signal(struct cvm* const cvm) {
  RCSW_FPC_NV(ERROR, NULL != cvm);
  RCSW_CHECK(0 == condv_signal(&cvm->cv));
  return OK;

error:
  return ERROR;
} /* mt_cvm_signal() */

status_t cvm_wait(struct cvm* const cvm) {
  RCSW_FPC_NV(ERROR, NULL != cvm);
  RCSW_CHECK(0 == condv_wait(&cvm->cv, &cvm->mtx));
  return OK;

error:
  return ERROR;
} /* mt_cvm_wait() */

status_t cvm_timedwait(struct cvm* const cvm, const struct timespec* const to) {
  RCSW_FPC_NV(ERROR, NULL != cvm, NULL != to);
  struct timespec ts = { .tv_sec = 0, .tv_nsec = 0 };

  RCSW_CHECK(OK == time_ts_ref_conv(to, &ts));
  RCSW_CHECK(0 == condv_timedwait(&cvm->cv, &cvm->mtx, &ts));
  return OK;

error:
  return ERROR;
} /* struct cvmimedwait() */

status_t cvm_broadcast(struct cvm* const cvm) {
  RCSW_FPC_NV(ERROR, NULL != cvm);
  RCSW_CHECK(0 == condv_broadcast(&cvm->cv));
  return OK;

error:
  return ERROR;
} /* mt_cvm_broadcast() */

END_C_DECLS
