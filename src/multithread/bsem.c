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
#include "rcsw/multithread/bsem.h"

#include <errno.h>

#include "rcsw/core/alloc.h"
#include "rcsw/core/compilers.h"
#include "rcsw/core/flags.h"
#include "rcsw/core/fpc.h"
#include "rcsw/er/client.h"
#include "rcsw/utils/time.h"

/*******************************************************************************
 * Public API
 ******************************************************************************/
BEGIN_C_DECLS

struct bsem* bsem_init(struct bsem* const sem_in, uint32_t flags) {
  struct bsem* sem = rcsw_alloc(sem_in,
                                sizeof(struct bsem),
                                flags & (RCSW_NOALLOC_HANDLE | RCSW_ZALLOC));
  if (NULL == sem) {
    errno = ENOMEM;
    return NULL;
  }
  sem->flags     = flags;
  sem->val       = 1;
  sem->flush_gen = 0;

  if (NULL == mutex_init(&sem->mtx, RCSW_NOALLOC_HANDLE)) {
    goto free_handle;
  }
  if (NULL == condv_init(&sem->cv, RCSW_NOALLOC_HANDLE)) {
    goto destroy_mutex;
  }
  return sem;

  /* Undo only what was initialized; errno is already set */
destroy_mutex:
  mutex_destroy(&sem->mtx);
free_handle:
  rcsw_free(sem, flags & RCSW_NOALLOC_HANDLE);
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
  /* Binary: posting an available semaphore leaves it available */
  if (0 == sem->val) {
    sem->val = 1;
    condv_signal(&sem->cv);
  }
  RCSW_CHECK(OK == mutex_unlock(&sem->mtx));
  return OK;

error:
  return ERROR;
} /* bsem_post() */

status_t bsem_timedwait(struct bsem* const sem, const struct timespec* const to) {
  RCSW_FPC_NV(ERROR, NULL != sem, NULL != to);

  /* One deadline for the whole wait, however many spurious wakeups */
  struct timespec deadline = {.tv_sec = 0, .tv_nsec = 0};
  RCSW_CHECK(OK == utils_ts_make_abs(to, &deadline));

  RCSW_CHECK(OK == mutex_lock(&sem->mtx));
  uint32_t gen = sem->flush_gen;
  while (0 == sem->val && gen == sem->flush_gen) {
    if (OK != condv_timedwait_abs(&sem->cv, &sem->mtx, &deadline)) {
      mutex_unlock(&sem->mtx);
      return ERROR; /* errno set (ETIMEDOUT on timeout) */
    }
  }
  if (gen == sem->flush_gen) {
    sem->val = 0; /* acquired */
  }
  RCSW_CHECK(OK == mutex_unlock(&sem->mtx));
  return OK;

error:
  return ERROR;
} /* bsem_timedwait() */

status_t bsem_wait(struct bsem* const sem) {
  RCSW_FPC_NV(ERROR, NULL != sem);

  RCSW_CHECK(OK == mutex_lock(&sem->mtx));
  uint32_t gen = sem->flush_gen;
  while (0 == sem->val && gen == sem->flush_gen) {
    if (OK != condv_wait(&sem->cv, &sem->mtx)) {
      mutex_unlock(&sem->mtx);
      return ERROR;
    }
  }
  if (gen == sem->flush_gen) {
    sem->val = 0; /* acquired */
  }
  RCSW_CHECK(OK == mutex_unlock(&sem->mtx));
  return OK;

error:
  return ERROR;
} /* bsem_wait() */

status_t bsem_flush(struct bsem* const sem) {
  RCSW_FPC_NV(ERROR, NULL != sem);

  /* Release every current waiter, and leave the semaphore available */
  RCSW_CHECK(OK == mutex_lock(&sem->mtx));
  sem->flush_gen++;
  sem->val = 1;
  condv_broadcast(&sem->cv);
  RCSW_CHECK(OK == mutex_unlock(&sem->mtx));
  return OK;

error:
  return ERROR;
} /* bsem_flush() */

END_C_DECLS
