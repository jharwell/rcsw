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

#define RCSW_ER_MODNAME "rcsw.ds.fifo"
#define RCSW_ER_MODID ekLOG4CL_DS_FIFO
#include "rcsw/er/client.h"
#include "rcsw/common/alloc.h"

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

struct fifo* fifo_init(struct fifo* fifo_in,
                       const struct fifo_params* const params) {
  RCSW_FPC_NV(NULL,
              params != NULL,
              params->max_elts > 0,
              params->elt_size > 0);
  RCSW_ER_MODULE_INIT();

  struct fifo* fifo = rcsw_alloc(fifo_in,
                                 sizeof(struct fifo),
                                 params->flags & RCSW_NOALLOC_HANDLE);

  RCSW_CHECK_PTR(fifo);
  fifo->flags = params->flags;

  struct rbuffer_params rb_params = { .printe = params->printe,
                                 .cmpe = NULL,
                                 .elt_size = params->elt_size,
                                 .max_elts = params->max_elts,
                                 .elements = params->elements,
                                 .flags = params->flags };
  rb_params.flags |= (RCSW_NOALLOC_HANDLE | RCSW_DS_RBUFFER_AS_FIFO);
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

  rcsw_free(fifo, fifo->flags & RCSW_NOALLOC_HANDLE);
} /* fifo_destroy() */

status_t fifo_add(struct fifo* const fifo, const void* const e) {
  RCSW_FPC_NV(ERROR, NULL != fifo, NULL != e);
  return rbuffer_add(&fifo->rb, e);
} /* fifo_add() */

status_t fifo_remove(struct fifo* const fifo, void* const e) {
  RCSW_FPC_NV(ERROR, NULL != fifo);
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
    DPRINTF(RCSW_ER_MODNAME " : < NULL >\n");
    return;
  }
  rbuffer_print(&fifo->rb);
} /* fifo_print() */

END_C_DECLS
