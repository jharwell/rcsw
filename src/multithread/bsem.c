/**
 * \file mt_bsem.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/multithread/bsem.h"

#include "rcsw/er/client.h"
#include "rcsw/common/fpc.h"
#include "rcsw/rcsw.h"
#include "rcsw/common/alloc.h"

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

struct bsem* bsem_init(struct bsem* const sem_in, uint32_t flags) {
  struct bsem* sem = rcsw_alloc(sem_in,
                                sizeof(struct bsem),
                                flags & RCSW_NOALLOC_HANDLE);
  RCSW_CHECK_PTR(sem);
  sem->flags = flags;

  RCSW_CHECK(NULL != mutex_init(&sem->mtx, RCSW_NOALLOC_HANDLE));
  RCSW_CHECK(NULL != condv_init(&sem->cv, RCSW_NOALLOC_HANDLE));
  sem->val = 1;
  return sem;

error:
  bsem_destroy(sem);
  return NULL;
} /* bsem_init() */

void bsem_destroy(struct bsem* const sem) {
  RCSW_FPC_V(NULL != sem);

  mutex_destroy(&sem->mtx);
  condv_destroy(&sem->cv);
  rcsw_free(sem, sem->flags & RCSW_NOALLOC_HANDLE);
} /* bsem_destroy() */

status_t bsem_post(struct bsem* const sem) {
  RCSW_FPC_NV(ERROR, NULL != sem);

  RCSW_CHECK(OK == mutex_lock(&sem->mtx));
  RCSW_CHECK(1 != sem->val);

  sem->val += 1;

  RCSW_CHECK(OK == condv_signal(&sem->cv));
  RCSW_CHECK(OK == mutex_unlock(&sem->mtx));

error:
  return ERROR;
} /* bsem_post() */

status_t bsem_timedwait(struct bsem* const sem,
                        const struct timespec* const to) {
  RCSW_FPC_NV(ERROR, NULL != sem, NULL != to);

  RCSW_CHECK(OK == mutex_lock(&sem->mtx));
  while (0 == sem->val) {
    condv_timedwait(&sem->cv, &sem->mtx, to);
  }
  sem->val -= 1;
  RCSW_CHECK(OK == mutex_unlock(&sem->mtx));
  return OK;

error:
  return ERROR;
} /* struct bsemimedwait() */

status_t bsem_wait(struct bsem* const sem) {
  RCSW_FPC_NV(ERROR, NULL != sem);

  RCSW_CHECK(OK == mutex_lock(&sem->mtx));
  while (0 == sem->val) {
    condv_wait(&sem->cv, &sem->mtx);
  }
  sem->val -= 1;
  RCSW_CHECK(OK == mutex_unlock(&sem->mtx));
  return OK;

error:
  return ERROR;
} /* bsem_wait() */

status_t bsem_flush(struct bsem* const sem) {
  RCSW_FPC_NV(ERROR, NULL != sem);

  RCSW_CHECK(OK == mutex_lock(&sem->mtx));
  RCSW_CHECK(1 != sem->val);
  sem->val += 1;
  RCSW_CHECK(OK == condv_broadcast(&sem->cv));
  RCSW_CHECK(OK == mutex_unlock(&sem->mtx));
  return OK;

error:
  return ERROR;
} /* bsem_flush() */

END_C_DECLS
