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
#include "rcsw/multithread/mutex.h"

#include <errno.h>

#include "rcsw/core/alloc.h"
#include "rcsw/core/flags.h"
#include "rcsw/core/fpc.h"
#include "rcsw/er/client.h"

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

struct mutex* mutex_init(struct mutex* mutex_in, uint32_t flags) {
  struct mutex* mutex = rcsw_alloc(mutex_in,
                                   sizeof(struct mutex),
                                   flags & (RCSW_NOALLOC_HANDLE | RCSW_ZALLOC));
  if (NULL == mutex) {
    errno = ENOMEM;
    return NULL;
  }
  mutex->flags = flags;
  if (0 != rcsw_pthread_rc(pthread_mutex_init(&mutex->impl, NULL))) {
    /* never initialized: don't pthread_mutex_destroy() it */
    rcsw_free(mutex, flags & RCSW_NOALLOC_HANDLE);
    return NULL;
  }
  return mutex;
} /* mutex_init() */

void mutex_destroy(struct mutex* mutex) {
  RCSW_FPC_V(NULL != mutex);

  pthread_mutex_destroy(&mutex->impl);
  rcsw_free(mutex, mutex->flags & RCSW_NOALLOC_HANDLE);
} /* mutex_destroy() */

status_t mutex_lock(struct mutex* mutex) {
  RCSW_FPC_NV(ERROR, NULL != mutex);

  RCSW_CHECK(0 == rcsw_pthread_rc(pthread_mutex_lock(&mutex->impl)));
  return OK;

error:
  return ERROR;
} /* mutex_lock() */

status_t mutex_unlock(struct mutex* mutex) {
  RCSW_FPC_NV(ERROR, NULL != mutex);
  RCSW_CHECK(0 == rcsw_pthread_rc(pthread_mutex_unlock(&mutex->impl)));
  return OK;

error:
  return ERROR;
} /* mutex_unlock() */

END_C_DECLS
