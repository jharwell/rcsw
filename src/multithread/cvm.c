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
#include "rcsw/multithread/cvm.h"

#include "rcsw/core/alloc.h"
#include "rcsw/core/fpc.h"
#include "rcsw/er/client.h"
#include "rcsw/utils/time.h"

/*******************************************************************************
 * Public API
 ******************************************************************************/
BEGIN_C_DECLS

struct cvm* cvm_init(struct cvm* const cvm_in, uint32_t flags) {
  struct cvm* cvm =
    rcsw_alloc(cvm_in, sizeof(struct cvm), flags & RCSW_NOALLOC_HANDLE);

  RCSW_CHECK_PTR(cvm);
  cvm->flags = flags;

  RCSW_CHECK(OK == condv_init(&cvm->cv, cvm->flags));
  RCSW_CHECK(OK == mutex_init(&cvm->mtx, cvm->flags));
  return cvm;

error:
  cvm_destroy(cvm);
  return NULL;
}

void cvm_destroy(struct cvm* const cvm) {
  RCSW_FPC_V(NULL != cvm);

  condv_destroy(&cvm->cv);
  mutex_destroy(&cvm->mtx);
  rcsw_free(cvm, cvm->flags & RCSW_NOALLOC_HANDLE);
}

status_t cvm_signal(struct cvm* const cvm) {
  RCSW_FPC_NV(ERROR, NULL != cvm);
  RCSW_CHECK(OK == condv_signal(&cvm->cv));
  return OK;

error:
  return ERROR;
}

status_t cvm_wait(struct cvm* const cvm) {
  RCSW_FPC_NV(ERROR, NULL != cvm);
  RCSW_CHECK(OK == condv_wait(&cvm->cv, &cvm->mtx));
  return OK;

error:
  return ERROR;
}

status_t cvm_timedwait(struct cvm* const cvm, const struct timespec* const to) {
  RCSW_FPC_NV(ERROR, NULL != cvm, NULL != to);
  struct timespec ts = {.tv_sec = 0, .tv_nsec = 0};

  RCSW_CHECK(OK == utils_ts_make_abs(to, &ts));
  RCSW_CHECK(OK == condv_timedwait(&cvm->cv, &cvm->mtx, &ts));
  return OK;

error:
  return ERROR;
}

status_t cvm_broadcast(struct cvm* const cvm) {
  RCSW_FPC_NV(ERROR, NULL != cvm);
  RCSW_CHECK(OK == condv_broadcast(&cvm->cv));
  return OK;

error:
  return ERROR;
}

END_C_DECLS
