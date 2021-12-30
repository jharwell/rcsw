/**
 * @file fifo.c
 *
 * @copyright 2017 John Harwell, All rights reserved.
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
#include "rcsw/ds/fifo.h"
#include "rcsw/common/dbg.h"

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

struct fifo* fifo_init(struct fifo* fifo_in,
                       const struct ds_params* const params) {
  RCSW_FPC_NV(NULL,
            params != NULL,
            params->tag == DS_FIFO,
            params->max_elts > 0,
            params->el_size > 0);

  struct fifo* fifo = NULL;
  if (params->flags & DS_APP_DOMAIN_HANDLE) {
    RCSW_CHECK_PTR(fifo_in);
    fifo = fifo_in;
  } else {
    fifo = malloc(sizeof(struct fifo));
    RCSW_CHECK_PTR(fifo);
  }
  fifo->flags = params->flags;

  struct ds_params rb_params = {.printe = params->printe,
                                .cmpe = params->cmpe,
                                .el_size = params->el_size,
                                .max_elts = params->max_elts,
                                .tag = DS_RBUFFER,
                                .elements = params->elements,
                                .flags = params->flags};
  rb_params.flags |= (DS_APP_DOMAIN_HANDLE | DS_RBUFFER_AS_FIFO);
  RCSW_CHECK(NULL != rbuffer_init(&fifo->rb, &rb_params));
  return fifo;

error:
  fifo_destroy(fifo);
  errno = EAGAIN;
  return NULL;
} /* fifo_init() */

void fifo_destroy(struct fifo* const fifo) {
  RCSW_FPC_V(NULL != fifo);

  rbuffer_destroy(&fifo->rb);
  if (!(fifo->flags & DS_APP_DOMAIN_HANDLE)) {
    free(fifo);
  }
} /* fifo_destroy() */

status_t fifo_enq(struct fifo* const fifo, const void* const e) {
  RCSW_FPC_NV(ERROR, NULL != fifo, NULL != e);
  return rbuffer_add(&fifo->rb, e);
} /* fifo_enq() */

status_t fifo_deq(struct fifo* const fifo, void* const e) {
  RCSW_FPC_NV(ERROR, NULL != fifo, NULL != e);
  return rbuffer_remove(&fifo->rb, e);
} /* fifo_deq() */

status_t fifo_clear(struct fifo* const fifo) {
  RCSW_FPC_NV(ERROR, NULL != fifo);
  return rbuffer_clear(&fifo->rb);
} /* fifo_clear() */

status_t fifo_map(struct fifo* const fifo, void (*f)(void* e)) {
  RCSW_FPC_NV(ERROR, NULL != fifo);
  return rbuffer_map(&fifo->rb, f);
} /* fifo_map() */

status_t fifo_inject(struct fifo* const fifo,
                     void (*f)(void* e, void* res),
                     void* result) {
  RCSW_FPC_NV(ERROR, NULL != fifo);
  return rbuffer_inject(&fifo->rb, f, result);
} /* fifo_inject() */

void fifo_print(struct fifo* const fifo) {
  if (NULL == fifo) {
    DPRINTF("NULL FIFO\n");
    return;
  }
  rbuffer_print(&fifo->rb);
} /* fifo_print() */

END_C_DECLS
