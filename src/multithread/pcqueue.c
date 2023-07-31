/**
 * \file pcqueue.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/multithread/pcqueue.h"

#include "rcsw/er/client.h"
#include "rcsw/common/fpc.h"
#include "rcsw/common/alloc.h"

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

struct pcqueue* pcqueue_init(struct pcqueue* queue_in,
                             const struct pcqueue_params* const params) {
  RCSW_FPC_NV(NULL, NULL != params, params->max_elts > 0, params->elt_size > 0);

  struct pcqueue* queue = rcsw_alloc(queue_in,
                                     sizeof(struct pcqueue),
                                     params->flags & RCSW_NOALLOC_HANDLE);

  RCSW_CHECK_PTR(queue);

  queue->flags = params->flags;

  /* create FIFO */
  struct fifo_params impl_params = { .max_elts = params->max_elts,
                                     .printe = NULL,
                                     .elt_size = params->elt_size,
                                     .elements = params->elements,
                                     .flags = params->flags };
  impl_params.flags |= RCSW_NOALLOC_HANDLE;

  RCSW_CHECK(NULL != fifo_init(&queue->fifo, &impl_params));

  /* all slots available initially */
  RCSW_CHECK_PTR(csem_init(&queue->slots_avail,
                           params->max_elts,
                           RCSW_NOALLOC_HANDLE));
  RCSW_CHECK_PTR(csem_init(&queue->slots_inuse, 0, RCSW_NOALLOC_HANDLE));
  RCSW_CHECK_PTR(mutex_init(&queue->mutex, RCSW_NOALLOC_HANDLE));
  return queue;

error:
  pcqueue_destroy(queue);
  errno = EAGAIN;
  return NULL;
} /* pcqueue_init() */

void pcqueue_destroy(struct pcqueue* const queue) {
  RCSW_FPC_V(NULL != queue);

  fifo_destroy(&queue->fifo);

  mutex_destroy(&queue->mutex);
  csem_destroy(&queue->slots_avail);
  csem_destroy(&queue->slots_inuse);

  rcsw_free(queue, queue->flags & RCSW_NOALLOC_HANDLE);
} /* pcqueue_destroy() */

status_t pcqueue_push(struct pcqueue* const queue, const void* const e) {
  RCSW_FPC_NV(ERROR, NULL != queue, NULL != e);

  csem_wait(&queue->slots_avail);

  mutex_lock(&queue->mutex);
  status_t rval = fifo_add(&queue->fifo, e);
  mutex_unlock(&queue->mutex);

  if (OK == rval) {
    csem_post(&queue->slots_inuse);
  }

  return rval;
} /* pcqueue_push() */

status_t pcqueue_pop(struct pcqueue* const queue, void* const e) {
  RCSW_FPC_NV(ERROR, NULL != queue);

  csem_wait(&queue->slots_inuse);

  mutex_lock(&queue->mutex);
  status_t rval = fifo_remove(&queue->fifo, e);
  mutex_unlock(&queue->mutex);

  if (OK == rval) {
    csem_post(&queue->slots_avail);
  }

  return rval;
} /* pcqueue_pop() */

status_t pcqueue_timedpop(struct pcqueue* const queue,
                            const struct timespec* const to,
                            void* const e) {
  RCSW_FPC_NV(ERROR, NULL != queue, NULL != to);

  RCSW_CHECK(OK == csem_timedwait(&queue->slots_inuse, to));

  mutex_lock(&queue->mutex);

  /*
   * We don't use a CHECK() macro here because we need to make sure we unlock
   * the mutex, and we can't do that AND have the csem_timedwait() call above
   * also use a CHECK() macro, because in one case the mutex is locked when the
   * check is called and in one it isn't.
   */
  status_t rval = fifo_remove(&queue->fifo, e);
  mutex_unlock(&queue->mutex);
  csem_post(&queue->slots_avail);

  return rval;

error:
  return ERROR;
} /* pcqueue_pop() */

status_t pcqueue_timedpeek(struct pcqueue* const queue,
                           const struct timespec* const to,
                           void** const e) {
  RCSW_FPC_NV(ERROR, NULL != queue, NULL != to, NULL != e);

  RCSW_CHECK(OK == csem_timedwait(&queue->slots_inuse, to));

  mutex_lock(&queue->mutex);

  *e = fifo_front(&queue->fifo);

  mutex_unlock(&queue->mutex);
  csem_post(&queue->slots_inuse);

  return OK;

error:
  *e = NULL;
  return ERROR;
} /* pcqueue_timedpeek() */

status_t pcqueue_peek(struct pcqueue* const queue,
                      void** const e) {
  RCSW_FPC_NV(ERROR, NULL != queue, NULL != e);

  csem_wait(&queue->slots_inuse);
  mutex_lock(&queue->mutex);

  *e = fifo_front(&queue->fifo);

  mutex_unlock(&queue->mutex);
  csem_post(&queue->slots_inuse);

  return OK;
} /* pcqueue_peek() */

END_C_DECLS
