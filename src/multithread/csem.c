/**
 * \file csem.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/multithread/csem.h"

#include "rcsw/common/fpc.h"
#include "rcsw/er/client.h"
#include "rcsw/rcsw.h"
#include "rcsw/utils/time.h"
#include "rcsw/common/alloc.h"
#include "rcsw/common/flags.h"

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

struct csem* csem_init(struct csem* const sem_in, size_t value, uint32_t flags) {
  struct csem* sem = rcsw_alloc(sem_in,
                                sizeof(struct csem),
                                flags & RCSW_NOALLOC_HANDLE);

  RCSW_CHECK_PTR(sem);
  sem->flags = flags;
  RCSW_CHECK(0 == sem_init(&sem->impl,
                           0, /* shared between threads */
                           (unsigned int)value));
  return sem;

error:
  return NULL;
} /* csem_init() */

void csem_destroy(struct csem* sem) {
  RCSW_FPC_V(NULL != sem);

  sem_destroy(&sem->impl);
  rcsw_free(sem, sem->flags & RCSW_NOALLOC_HANDLE);
} /* csem_destroy() */

status_t csem_wait(struct csem* sem) {
  RCSW_FPC_NV(ERROR, NULL != sem);
  RCSW_CHECK(0 == sem_wait(&sem->impl));
  return OK;

error:
  return ERROR;
} /* csem_wait() */

status_t csem_trywait(struct csem* sem) {
  RCSW_FPC_NV(ERROR, NULL != sem);
  RCSW_CHECK(0 == sem_trywait(&sem->impl));
  return OK;

error:
  return ERROR;
} /* csem_wait() */

status_t csem_timedwait(struct csem* const sem,
                           const struct timespec* const to) {
  RCSW_FPC_NV(ERROR, NULL != sem, NULL != to);
  struct timespec ts = { .tv_sec = 0, .tv_nsec = 0 };
  RCSW_CHECK(OK == time_ts_make_abs(to, &ts));
  RCSW_CHECK(0 == sem_timedwait(&sem->impl, &ts));

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
} /* csem_post() */

END_C_DECLS
