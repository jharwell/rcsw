/**
 * \file mt_mutex.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/multithread/mutex.h"

#include "rcsw/common/fpc.h"
#include "rcsw/er/client.h"
#include "rcsw/common/alloc.h"

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

struct mutex* mutex_init(struct mutex* mutex_in, uint32_t flags) {
  struct mutex* mutex = rcsw_alloc(mutex_in,
                                   sizeof(struct mutex),
                                   flags & RCSW_NOALLOC_HANDLE);

  RCSW_CHECK_PTR(mutex);
  mutex->flags = flags;
  RCSW_CHECK(0 == pthread_mutex_init(&mutex->impl, NULL));
  return mutex;

error:
  mutex_destroy(mutex);
  return NULL;
} /* mutex_init() */

void mutex_destroy(struct mutex* mutex) {
  RCSW_FPC_V(NULL != mutex);

  pthread_mutex_destroy(&mutex->impl);
  rcsw_free(mutex, mutex->flags & RCSW_NOALLOC_HANDLE);
} /* mutex_destroy() */

status_t mutex_lock(struct mutex* mutex) {
  RCSW_FPC_NV(ERROR, NULL != mutex);

  RCSW_CHECK(0 == pthread_mutex_lock(&mutex->impl));
  return OK;

error:
  return ERROR;
} /* mutex_lock() */

status_t mutex_unlock(struct mutex* mutex) {
  RCSW_FPC_NV(ERROR, NULL != mutex);
  RCSW_CHECK(0 == pthread_mutex_unlock(&mutex->impl));
  return OK;

error:
  return ERROR;
} /* mutex_unlock() */

END_C_DECLS
