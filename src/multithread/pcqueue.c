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

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

struct pcqueue* pcqueue_init(struct pcqueue* queue_in,
                             const struct pcqueue_params* const params) {
  RCSW_FPC_NV(NULL, NULL != params, params->max_elts > 0, params->elt_size > 0);

  struct pcqueue* queue = NULL;

  if (params->flags & RCSW_NOALLOC_HANDLE) {
    RCSW_CHECK_PTR(queue_in);
    queue = queue_in;
  } else {
    queue = calloc(1, sizeof(struct pcqueue));
    RCSW_CHECK_PTR(queue);
  }
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
  RCSW_CHECK_PTR(csem_init(&queue->empty,
                           params->max_elts,
                           RCSW_NOALLOC_HANDLE));
  RCSW_CHECK_PTR(csem_init(&queue->full, 0, RCSW_NOALLOC_HANDLE));
  RCSW_CHECK_PTR(mutex_init(&queue->mutex, RCSW_NOALLOC_HANDLE));
  return queue;

error:
  pcqueue_destroy(queue);
  errno = EAGAIN;
  return NULL;
} /* pcqueue_init() */

void pcqueue_destroy(struct pcqueue* const queue) {
  RCSW_FPC_V(NULL != queue);

  if (!(queue->flags & RCSW_NOALLOC_HANDLE)) {
    free(queue);
  }
} /* pcqueue_destroy() */

status_t pcqueue_push(struct pcqueue* const queue, const void* const e) {
  RCSW_FPC_NV(ERROR, NULL != queue, NULL != e);

  status_t rval = ERROR;

  csem_wait(&queue->empty);
  mutex_lock(&queue->mutex);
  RCSW_CHECK(OK == fifo_add(&queue->fifo, e));
  rval = OK;

error:
  mutex_unlock(&queue->mutex);
  csem_post(&queue->full);
  return rval;
} /* pcqueue_push() */

status_t pcqueue_pop(struct pcqueue* const queue, void* const e) {
  RCSW_FPC_NV(ERROR, NULL != queue);

  status_t rval = ERROR;

  csem_wait(&queue->full);
  mutex_lock(&queue->mutex);

  RCSW_CHECK(OK == fifo_remove(&queue->fifo, e));
  rval = OK;

error:
  mutex_unlock(&queue->mutex);
  csem_post(&queue->empty);
  return rval;
} /* pcqueue_pop() */

status_t pcqueue_timed_pop(struct pcqueue* const queue,
                            const struct timespec* const to,
                            void* const e) {
  RCSW_FPC_NV(ERROR, NULL != queue);

  status_t rval = ERROR;

  RCSW_CHECK(OK == csem_timedwait(&queue->full, to));
  mutex_lock(&queue->mutex);

  RCSW_CHECK(OK == fifo_remove(&queue->fifo, e));
  rval = OK;

error:
  mutex_unlock(&queue->mutex);
  csem_post(&queue->empty);
  return rval;
} /* pcqueue_pop() */

void* pcqueue_peek(struct pcqueue* const queue) {
  RCSW_FPC_NV(NULL, NULL != queue);

  uint8_t* ret = NULL;

  RCSW_CHECK(-1 != csem_trywait(&queue->full));
  mutex_lock(&queue->mutex);
  ret = fifo_front(&queue->fifo);
  mutex_unlock(&queue->mutex);

error:
  return ret;
} /* pcqueue_pop() */

END_C_DECLS
