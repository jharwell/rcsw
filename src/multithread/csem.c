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
#include "rcsw/multithread/csem.h"

#include <errno.h>
#include <limits.h>
#include <semaphore.h>

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

struct csem* csem_init(struct csem* const sem_in, size_t value, uint32_t flags) {
  RCSW_FPC_NV(NULL, value <= SEM_VALUE_MAX);
  struct csem* sem = rcsw_alloc(sem_in,
                                sizeof(struct csem),
                                flags & (RCSW_NOALLOC_HANDLE | RCSW_ZALLOC));
  if (NULL == sem) {
    errno = ENOMEM;
    return NULL;
  }
  sem->flags = flags;
  if (0 !=
      sem_init(&sem->impl, 0 /* shared between threads */, (unsigned)value)) {
    rcsw_free(sem, flags & RCSW_NOALLOC_HANDLE); /* errno set by sem_init() */
    return NULL;
  }
  return sem;
} /* csem_init() */

void csem_destroy(struct csem* sem) {
  RCSW_FPC_V(NULL != sem);

  sem_destroy(&sem->impl);
  rcsw_free(sem, sem->flags & RCSW_NOALLOC_HANDLE);
}

status_t csem_wait(struct csem* sem) {
  RCSW_FPC_NV(ERROR, NULL != sem);

  /* A signal interrupting the wait is not a failure: keep waiting */
  int rc;
  do {
    rc = sem_wait(&sem->impl);
  } while (0 != rc && EINTR == errno);
  RCSW_CHECK(0 == rc);
  return OK;

error:
  return ERROR;
}

status_t csem_trywait(struct csem* sem) {
  RCSW_FPC_NV(ERROR, NULL != sem);
  RCSW_CHECK(0 == sem_trywait(&sem->impl));
  return OK;

error:
  return ERROR;
}

status_t csem_timedwait(struct csem* const sem, const struct timespec* const to) {
  RCSW_FPC_NV(ERROR, NULL != sem, NULL != to);
  struct timespec ts = {.tv_sec = 0, .tv_nsec = 0};
  RCSW_CHECK(OK == utils_ts_make_abs(to, &ts));
  return csem_timedwait_abs(sem, &ts);

error:
  return ERROR;
}

status_t csem_timedwait_abs(struct csem* const           sem,
                            const struct timespec* const to) {
  RCSW_FPC_NV(ERROR, NULL != sem, NULL != to);

  /* Retry after a signal; the absolute deadline stays fixed */
  int rc;
  do {
    rc = sem_timedwait(&sem->impl, to);
  } while (0 != rc && EINTR == errno);
  RCSW_CHECK(0 == rc);
  return OK;

error:
  return ERROR;
}

status_t csem_post(struct csem* sem) {
  RCSW_FPC_NV(ERROR, NULL != sem);
  RCSW_CHECK(0 == sem_post(&sem->impl));
  return OK;

error:
  return ERROR;
}

END_C_DECLS
