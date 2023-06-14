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
#include "rcsw/multithread/mt_mutex.h"

#include "rcsw/common/dbg.h"
#include "rcsw/common/fpc.h"

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

mt_mutex_t* mt_mutex_init(mt_mutex_t* mutex_in, uint32_t flags) {
  mt_mutex_t* mutex = NULL;
  if (flags & MT_APP_DOMAIN_MEM) {
    mutex = mutex_in;
  } else {
    mutex = calloc(1, sizeof(mt_mutex_t));
  }

  RCSW_CHECK_PTR(mutex);
  mutex->flags = flags;
  RCSW_CHECK(0 == pthread_mutex_init(&mutex->mutex, NULL));
  return mutex;

error:
  mt_mutex_destroy(mutex);
  return NULL;
} /* mt_mutex_init() */

void mt_mutex_destroy(mt_mutex_t* mutex) {
  RCSW_FPC_V(NULL != mutex);

  pthread_mutex_destroy(&mutex->mutex);
  if (mutex->flags & MT_APP_DOMAIN_MEM) {
    free(mutex);
  }
} /* mt_mutex_destroy() */

status_t mt_mutex_lock(mt_mutex_t* mutex) {
  RCSW_FPC_NV(ERROR, NULL != mutex);
  RCSW_CHECK(0 == pthread_mutex_lock(&mutex->mutex));
  return OK;

error:
  return ERROR;
} /* mt_mutex_lock() */

status_t mt_mutex_unlock(mt_mutex_t* mutex) {
  RCSW_FPC_NV(ERROR, NULL != mutex);
  RCSW_CHECK(0 == pthread_mutex_unlock(&mutex->mutex));
  return OK;

error:
  return ERROR;
} /* mt_mutex_unlock() */

END_C_DECLS
