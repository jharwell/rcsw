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
#include "rcsw/multithread/mt_bsem.h"

#include "rcsw/common/dbg.h"
#include "rcsw/common/fpc.h"
#include "rcsw/rcsw.h"

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

mt_bsem_t* mt_bsem_init(mt_bsem_t* const sem_in, uint32_t flags) {
  mt_bsem_t* sem_p = NULL;
  if (flags & MT_APP_DOMAIN_MEM) {
    sem_p = sem_in;
  } else {
    sem_p = calloc(1, sizeof(mt_bsem_t));
  }
  RCSW_CHECK_PTR(sem_p);
  sem_p->flags = flags;

  RCSW_CHECK(NULL != mt_mutex_init(&sem_p->mutex, MT_APP_DOMAIN_MEM));
  RCSW_CHECK(NULL != mt_cond_init(&sem_p->cv, MT_APP_DOMAIN_MEM));
  sem_p->val = 1;
  return sem_p;

error:
  mt_bsem_destroy(sem_p);
  return NULL;
} /* mt_bsem_init() */

void mt_bsem_destroy(mt_bsem_t* const sem_p) {
  RCSW_FPC_V(NULL != sem_p);

  mt_mutex_destroy(&sem_p->mutex);
  mt_cond_destroy(&sem_p->cv);
  if (sem_p->flags & MT_APP_DOMAIN_MEM) {
    free(sem_p);
  }
} /* mt_bsem_destroy() */

status_t mt_bsem_post(mt_bsem_t* const sem_p) {
  RCSW_FPC_NV(ERROR, NULL != sem_p);
  RCSW_CHECK(OK == mt_mutex_lock(&sem_p->mutex));
  RCSW_CHECK(1 != sem_p->val);
  sem_p->val += 1;
  RCSW_CHECK(OK == mt_cond_signal(&sem_p->cv));
  RCSW_CHECK(OK == mt_mutex_unlock(&sem_p->mutex));

error:
  return ERROR;
} /* mt_bsem_post() */

status_t mt_bsem_timedwait(mt_bsem_t* const sem_p,
                           const struct timespec* const to) {
  RCSW_FPC_NV(ERROR, NULL != sem_p, NULL != to);

  RCSW_CHECK(OK == mt_mutex_lock(&sem_p->mutex));
  while (0 == sem_p->val) {
    mt_cond_timedwait(&sem_p->cv, &sem_p->mutex, to);
  }
  sem_p->val -= 1;
  RCSW_CHECK(OK == mt_mutex_unlock(&sem_p->mutex));
  return OK;

error:
  return ERROR;
} /* mt_bsem_timedwait() */

status_t mt_bsem_wait(mt_bsem_t* const sem_p) {
  RCSW_FPC_NV(ERROR, NULL != sem_p);

  RCSW_CHECK(OK == mt_mutex_lock(&sem_p->mutex));
  while (0 == sem_p->val) {
    mt_cond_wait(&sem_p->cv, &sem_p->mutex);
  }
  sem_p->val -= 1;
  RCSW_CHECK(OK == mt_mutex_unlock(&sem_p->mutex));
  return OK;

error:
  return ERROR;
} /* mt_bsem_wait() */

status_t mt_bsem_flush(mt_bsem_t* const sem_p) {
  RCSW_FPC_NV(ERROR, NULL != sem_p);

  RCSW_CHECK(OK == mt_mutex_lock(&sem_p->mutex));
  RCSW_CHECK(1 != sem_p->val);
  sem_p->val += 1;
  RCSW_CHECK(OK == mt_cond_broadcast(&sem_p->cv));
  RCSW_CHECK(OK == mt_mutex_unlock(&sem_p->mutex));
  return OK;

error:
  return ERROR;
} /* mt_bsem_flush() */

END_C_DECLS
