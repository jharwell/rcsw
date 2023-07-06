/**
 * \file rdwrlock.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/multithread/rdwrlock.h"

#define RCSW_ER_MODID M_MT_RDWRLOCK
#define RCSW_ER_MODNAME "rcsw.mt.rdwrl"
#include "rcsw/er/client.h"
#include "rcsw/common/fpc.h"
#include "rcsw/rcsw.h"

/*******************************************************************************
 * Private Functions
 ******************************************************************************/
static BEGIN_C_DECLS

void rdwrl_wr_exit(struct rdwrlock* const rdwr) {
  csem_post(&rdwr->access); /* release exclusive access to resource */
} /* rdwrl_wr_exit() */

static void rdwrl_wr_enter(struct rdwrlock* const rdwr) {
  /* get a place in line (ensure fairness) */
  csem_wait(&rdwr->order);

  /* request exclusive access to resource */
  csem_wait(&rdwr->access);

  /* we have gotten served, so release our place in line */
  csem_post(&rdwr->order);
} /* rdwrl_wr_enter() */

static status_t rdwrl_wr_timed_enter(struct rdwrlock* const rdwr,
                              const struct timespec* const to) {
  status_t rval = ERROR;

  /* get a place in line (ensure fairness) */
  csem_timedwait(&rdwr->order, to);

  /* request exclusive access to resource */
  RCSW_CHECK(OK == csem_timedwait(&rdwr->access, to));
  rval = OK;

error:
  /* we have gotten served, so release our place in line */
  csem_post(&rdwr->order);
  return rval;
} /* struct rdwrl_timed_wr_enter() */

static void rdwrl_rd_exit(struct rdwrlock* const rdwr) {
  /* we are going to modify the readers counter  */
  csem_wait(&rdwr->read);

  /* we are done; 1 less reader */
  rdwr->n_readers--;

  /* if we are the last reader */
  if (0 == rdwr->n_readers) {
    /* release exclusive access to the resource */
    csem_post(&rdwr->access);
  }
  /* finished updating # of readers */
  csem_post(&rdwr->read);
} /* rdwrl_rd_exit() */

static void rdwrl_rd_enter(struct rdwrlock* rdwr) {
  /* get a place in line (ensure fairness) */
  csem_wait(&rdwr->order);

  /* we are going to modify the readers counter */
  csem_wait(&rdwr->read);

  /* if we are the first reader */
  if (0 == rdwr->n_readers) {
    /* request exclusive access for readers */
    csem_wait(&rdwr->access);
  }
  ++rdwr->n_readers; /* 1 more reader */

  /* we have gotten served, so release our place in line */
  csem_post(&rdwr->order);

  /* finished updating # of readers */
  csem_post(&rdwr->read);
} /* rdwrl_rd_enter() */

static status_t rdwrl_rd_timed_enter(struct rdwrlock* const rdwr,
                                     const struct timespec* const to) {
  status_t rval = ERROR;

  /* get a place in line (ensure fairness) */
  csem_wait(&rdwr->order);

  /* we are going to modify the readers counter */
  csem_wait(&rdwr->read);

  /* if we are the first reader */
  if (0 == rdwr->n_readers) {
    /* request exclusive access for readers */
    RCSW_CHECK(OK == csem_timedwait(&rdwr->access, to));
  }
  ++rdwr->n_readers; /* 1 more reader */
  rval = OK;

error:
  /* we have gotten served, so release our place in line */
  csem_post(&rdwr->order);

  /* finished updating # of readers */
  csem_post(&rdwr->read);
  return rval;
} /* struct rdwrl_rd_timed_enter() */

/*******************************************************************************
 * API Functions
 ******************************************************************************/
status_t rdwrl_init(struct rdwrlock* const rdwr_in, uint32_t flags) {
  struct rdwrlock* rdwr = NULL;
  if (flags & RCSW_NOALLOC_HANDLE) {
    rdwr = rdwr_in;
  } else {
    rdwr = malloc(sizeof(struct rdwrlock));
  }
  RCSW_CHECK_PTR(rdwr);
  rdwr->flags = flags;

  rdwr->n_readers = 0;
  RCSW_CHECK(NULL != csem_init(&rdwr->order, 1, RCSW_NOALLOC_HANDLE));
  RCSW_CHECK(NULL != csem_init(&rdwr->access, 1, RCSW_NOALLOC_HANDLE));
  RCSW_CHECK(NULL != csem_init(&rdwr->read, 1, RCSW_NOALLOC_HANDLE));

  return OK;

error:
  rdwrl_destroy(rdwr);
  return ERROR;
} /* rdwrlinit() */

void rdwrl_destroy(struct rdwrlock* const rdwr) {
  RCSW_FPC_V(NULL != rdwr);

  csem_destroy(&rdwr->order);
  csem_destroy(&rdwr->access);
  csem_destroy(&rdwr->read);

  if (rdwr->flags & RCSW_NOALLOC_HANDLE) {
    free(rdwr);
  }
} /* rdwrl_destroy() */

void rdwrl_enter(struct rdwrlock *const rdwr, enum rdwrlock_scope scope) {
  switch (scope) {
    case ekSCOPE_RD:
      rdwrl_rd_enter(rdwr);
      break;
    case ekSCOPE_WR:
      rdwrl_wr_enter(rdwr);
      break;
    default:
      ER_SENTINEL("Bad privilege scope '%d' on enter", scope);
  } /* switch() */
error:
  return;
} /* rdwrl_enter() */

void rdwrl_exit(struct rdwrlock *const rdwr, enum rdwrlock_scope scope) {
  switch (scope) {
    case ekSCOPE_RD:
      rdwrl_rd_exit(rdwr);
      break;
    case ekSCOPE_WR:
      rdwrl_wr_exit(rdwr);
      break;
    default:
      ER_SENTINEL("Bad privilege scope '%d' on exit", scope);
  } /* switch() */
error:
  return;
} /* rdwrl_exit() */

status_t rdwrl_timed_enter(struct rdwrlock *const rdwr,
                           enum rdwrlock_scope scope,
                           const struct timespec* const to) {
  switch (scope) {
    case ekSCOPE_RD:
      return rdwrl_rd_timed_enter(rdwr, to);
      break;
    case ekSCOPE_WR:
      return rdwrl_wr_timed_enter(rdwr, to);
    default:
      ER_SENTINEL("Bad privilege scope '%d' on enter", scope);
  } /* switch() */

error:
  return ERROR;
} /* rdwrl_exit() */

END_C_DECLS
