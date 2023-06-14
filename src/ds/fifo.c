/**
 * \file fifo.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
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
              params->elt_size > 0);

  struct fifo* fifo = NULL;
  if (params->flags & RCSW_DS_NOALLOC_HANDLE) {
    RCSW_CHECK_PTR(fifo_in);
    fifo = fifo_in;
  } else {
    fifo = malloc(sizeof(struct fifo));
    RCSW_CHECK_PTR(fifo);
  }
  fifo->flags = params->flags;

  struct ds_params rb_params = { .printe = params->printe,
                                 .cmpe = params->cmpe,
                                 .elt_size = params->elt_size,
                                 .max_elts = params->max_elts,
                                 .tag = DS_RBUFFER,
                                 .elements = params->elements,
                                 .flags = params->flags };
  rb_params.flags |= (RCSW_DS_NOALLOC_HANDLE | RCSW_DS_RBUFFER_AS_FIFO);
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
  if (!(fifo->flags & RCSW_DS_NOALLOC_HANDLE)) {
    free(fifo);
  }
} /* fifo_destroy() */

status_t fifo_add(struct fifo* const fifo, const void* const e) {
  RCSW_FPC_NV(ERROR, NULL != fifo, NULL != e);
  return rbuffer_add(&fifo->rb, e);
} /* fifo_add() */

status_t fifo_remove(struct fifo* const fifo, void* const e) {
  RCSW_FPC_NV(ERROR, NULL != fifo, NULL != e);
  return rbuffer_remove(&fifo->rb, e);
} /* fifo_remove() */

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
