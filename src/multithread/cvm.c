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
#include "rcsw/multithread/cvm.h"

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

struct cvm* cvm_init(struct cvm* const cvm_in, uint32_t flags) {
  struct cvm* cvm = rcsw_alloc(cvm_in,
                               sizeof(struct cvm),
                               flags & (RCSW_NOALLOC_HANDLE | RCSW_ZALLOC));
  if (NULL == cvm) {
    errno = ENOMEM;
    return NULL;
  }
  cvm->flags = flags;

  /* The condv and mutex are embedded in the cvm: never separately allocated */
  if (NULL == condv_init(&cvm->cv, RCSW_NOALLOC_HANDLE)) {
    goto free_handle;
  }
  if (NULL == mutex_init(&cvm->mtx, RCSW_NOALLOC_HANDLE)) {
    goto destroy_cv;
  }
  return cvm;

  /* Undo only what was initialized; errno is already set */
destroy_cv:
  condv_destroy(&cvm->cv);
free_handle:
  rcsw_free(cvm, flags & RCSW_NOALLOC_HANDLE);
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

  RCSW_CHECK(OK == condv_timedwait(&cvm->cv, &cvm->mtx, to));
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
