/**
 * \file mt_csem.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/multithread/mt_rdwr_lock.h"

#include "rcsw/common/dbg.h"
#include "rcsw/common/fpc.h"
#include "rcsw/rcsw.h"

/*******************************************************************************
 * Functions
 ******************************************************************************/
BEGIN_C_DECLS

status_t mt_rdwr_lock_init(mt_rdwr_lock_t* const rdwr_in, uint32_t flags) {
  mt_rdwr_lock_t* rdwr = NULL;
  if (flags & MT_APP_DOMAIN_MEM) {
    rdwr = rdwr_in;
  } else {
    rdwr = malloc(sizeof(mt_rdwr_lock_t));
  }
  RCSW_CHECK_PTR(rdwr);
  rdwr->flags = flags;

  rdwr->n_readers = 0;
  RCSW_CHECK(NULL != mt_csem_init(&rdwr->order, FALSE, 1, MT_APP_DOMAIN_MEM));
  RCSW_CHECK(NULL != mt_csem_init(&rdwr->access, FALSE, 1, MT_APP_DOMAIN_MEM));
  RCSW_CHECK(NULL != mt_csem_init(&rdwr->read, FALSE, 1, MT_APP_DOMAIN_MEM));

  return OK;

error:
  mt_rdwr_lock_destroy(rdwr);
  return ERROR;
} /* mt_rdwr_lock_init() */

void mt_rdwr_lock_destroy(mt_rdwr_lock_t* const rdwr) {
  RCSW_FPC_V(NULL != rdwr);

  mt_csem_destroy(&rdwr->order);
  mt_csem_destroy(&rdwr->access);
  mt_csem_destroy(&rdwr->read);

  if (rdwr->flags & MT_APP_DOMAIN_MEM) {
    free(rdwr);
  }
} /* mt_rdwr_lock_destroy() */

void mt_rdwr_lock_wr_exit(mt_rdwr_lock_t* const rdwr) {
  mt_csem_post(&rdwr->access); /* release exclusive access to resource */
} /* mt_rdwr_lock_wr_exit() */

void mt_rdwr_lock_wr_enter(mt_rdwr_lock_t* const rdwr) {
  /* get a place in line (ensure fairness) */
  mt_csem_wait(&rdwr->order);

  /* request exclusive access to resource */
  mt_csem_wait(&rdwr->access);

  /* we have gotten served, so release our place in line */
  mt_csem_post(&rdwr->order);
} /* mt_rdwr_lock_wr_enter() */

status_t mt_rdwr_lock_timed_wr_enter(mt_rdwr_lock_t* const rdwr,
                                     const struct timespec* const to) {
  status_t rval = ERROR;

  /* get a place in line (ensure fairness) */
  mt_csem_timedwait(&rdwr->order, to);

  /* request exclusive access to resource */
  RCSW_CHECK(OK == mt_csem_timedwait(&rdwr->access, to));
  rval = OK;

error:
  /* we have gotten served, so release our place in line */
  mt_csem_post(&rdwr->order);
  return rval;
} /* mt_rdwr_lock_timed_wr_enter() */

void mt_rdwr_lock_rd_exit(mt_rdwr_lock_t* const rdwr) {
  /* we are going to modify the readers counter  */
  mt_csem_wait(&rdwr->read);

  /* we are done; 1 less reader */
  rdwr->n_readers--;

  /* if we are the last reader */
  if (0 == rdwr->n_readers) {
    /* release exclusive access to the resource */
    mt_csem_post(&rdwr->access);
  }
  /* finished updating # of readers */
  mt_csem_post(&rdwr->read);
} /* mt_rdwr_lock_rd_exit() */

void mt_rdwr_lock_rd_enter(mt_rdwr_lock_t* rdwr) {
  /* get a place in line (ensure fairness) */
  mt_csem_wait(&rdwr->order);

  /* we are going to modify the readers counter */
  mt_csem_wait(&rdwr->read);

  /* if we are the first reader */
  if (0 == rdwr->n_readers) {
    /* request exclusive access for readers */
    mt_csem_wait(&rdwr->access);
  }
  ++rdwr->n_readers; /* 1 more reader */

  /* we have gotten served, so release our place in line */
  mt_csem_post(&rdwr->order);

  /* finished updating # of readers */
  mt_csem_post(&rdwr->read);
} /* mt_rdwr_lock_rd_enter() */

status_t mt_rdwr_lock_timed_rd_enter(mt_rdwr_lock_t* const rdwr,
                                     const struct timespec* const to) {
  status_t rval = ERROR;

  /* get a place in line (ensure fairness) */
  mt_csem_wait(&rdwr->order);

  /* we are going to modify the readers counter */
  mt_csem_wait(&rdwr->read);

  /* if we are the first reader */
  if (0 == rdwr->n_readers) {
    /* request exclusive access for readers */
    RCSW_CHECK(OK == mt_csem_timedwait(&rdwr->access, to));
  }
  ++rdwr->n_readers; /* 1 more reader */
  rval = OK;

error:
  /* we have gotten served, so release our place in line */
  mt_csem_post(&rdwr->order);

  /* finished updating # of readers */
  mt_csem_post(&rdwr->read);
  return rval;
} /* mt_rdwr_lock_timed_rd_enter() */

END_C_DECLS
