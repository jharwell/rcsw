/**
 * \file mt_queue.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * This file is part of RCSW.
 *
 * RCSW is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * RCSW is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * RCSW.  If not, see <http://www.gnu.org/licenses/
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/multithread/mt_queue.h"
#include "rcsw/common/dbg.h"
#include "rcsw/common/fpc.h"

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

struct mt_queue* mt_queue_init(struct mt_queue* queue_in,
                               const struct mt_queue_params* const params) {
  RCSW_FPC_NV(NULL, NULL != params, params->max_elts > 0, params->el_size > 0);

  struct mt_queue* queue = NULL;

  if (params->flags & DS_APP_DOMAIN_HANDLE) {
    RCSW_CHECK_PTR(queue_in);
    queue = queue_in;
  } else {
    queue = calloc(1, sizeof(struct mt_queue));
    RCSW_CHECK_PTR(queue);
  }
  queue->flags = params->flags;

  /* create FIFO */
  struct ds_params fifo_params = {.max_elts = params->max_elts,
                                  .printe = NULL,
                                  .el_size = params->el_size,
                                  .tag = DS_FIFO,
                                  .elements = params->elements,
                                  .flags = params->flags};
  fifo_params.flags |= DS_APP_DOMAIN_HANDLE;

  RCSW_CHECK(NULL != fifo_init(&queue->fifo, &fifo_params));

  /* all slots available initially */
  RCSW_CHECK_PTR(
      mt_csem_init(&queue->empty, FALSE, params->max_elts, MT_APP_DOMAIN_MEM));
  RCSW_CHECK_PTR(mt_csem_init(&queue->full, FALSE, 0, MT_APP_DOMAIN_MEM));
  RCSW_CHECK_PTR(mt_mutex_init(&queue->mutex, MT_APP_DOMAIN_MEM));
  return queue;

error:
  mt_queue_destroy(queue);
  errno = EAGAIN;
  return NULL;
} /* mt_queue_init() */

void mt_queue_destroy(struct mt_queue* const queue) {
  RCSW_FPC_V(NULL != queue);

  if (!(queue->flags & DS_APP_DOMAIN_HANDLE)) {
    free(queue);
  }
} /* mt_queue_destroy() */

status_t mt_queue_push(struct mt_queue* const queue, const void* const e) {
  RCSW_FPC_NV(ERROR, NULL != queue, NULL != e);

  status_t rval = ERROR;

  mt_csem_wait(&queue->empty);
  mt_mutex_lock(&queue->mutex);
  RCSW_CHECK(OK == fifo_enq(&queue->fifo, e));
  rval = OK;

error:
  mt_mutex_unlock(&queue->mutex);
  mt_csem_post(&queue->full);
  return rval;
} /* mt_queue_push() */

status_t mt_queue_pop(struct mt_queue* const queue, void* const e) {
  RCSW_FPC_NV(ERROR, NULL != queue);

  status_t rval = ERROR;

  mt_csem_wait(&queue->full);
  mt_mutex_lock(&queue->mutex);

  RCSW_CHECK(OK == fifo_deq(&queue->fifo, e));
  rval = OK;

error:
  mt_mutex_unlock(&queue->mutex);
  mt_csem_post(&queue->empty);
  return rval;
} /* mt_queue_pop() */

status_t mt_queue_timed_pop(struct mt_queue* const queue,
                            const struct timespec* const to,
                            void* const e) {
  RCSW_FPC_NV(ERROR, NULL != queue);

  status_t rval = ERROR;

  RCSW_CHECK(OK == mt_csem_timedwait(&queue->full, to));
  mt_mutex_lock(&queue->mutex);

  RCSW_CHECK(OK == fifo_deq(&queue->fifo, e));
  rval = OK;

error:
  mt_mutex_unlock(&queue->mutex);
  mt_csem_post(&queue->empty);
  return rval;
} /* mt_queue_pop() */

void* mt_queue_peek(struct mt_queue* const queue) {
  RCSW_FPC_NV(NULL, NULL != queue);

  uint8_t* ret = NULL;

  RCSW_CHECK(-1 != mt_csem_trywait(&queue->full));
  mt_mutex_lock(&queue->mutex);
  ret = fifo_front(&queue->fifo);
  mt_mutex_unlock(&queue->mutex);

error:
  return ret;
} /* mt_queue_pop() */

END_C_DECLS
