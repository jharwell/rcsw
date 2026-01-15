/**
 * \file rbuffer.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/rbuffer.h"

#define RCSW_ER_MODID ekLOG4CL_DS_RBUFFER
#define RCSW_ER_MODNAME "rcsw.ds.rbuffer"
#include "rcsw/ds/ds.h"
#include "rcsw/er/client.h"
#include "rcsw/common/alloc.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/
#define RCSW_DS_RBUFFER_TYPE(rb) \
  (((rb)->flags & RCSW_DS_RBUFFER_AS_FIFO) ? "FIFO" : "RBUFFER")

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

struct rbuffer* rbuffer_init(struct rbuffer* rb_in,
                             const struct rbuffer_params* const params) {
  RCSW_FPC_NV(NULL,
              params != NULL,
              params->max_elts > 0,
              params->elt_size > 0);
  RCSW_ER_MODULE_INIT();

  struct rbuffer* rb = rcsw_alloc(rb_in,
                                  sizeof(struct rbuffer),
                                  params->flags & RCSW_NOALLOC_HANDLE);
  RCSW_CHECK_PTR(rb);
  rb->flags = params->flags;

  rb->elements = rcsw_alloc(params->elements,
                            params->max_elts * params->elt_size,
                            params->flags & RCSW_NOALLOC_DATA);

  RCSW_CHECK_PTR(rb->elements);

  rb->elt_size = params->elt_size;
  rb->printe = params->printe;
  rb->cmpe = params->cmpe;
  rb->start = 0;
  rb->current = 0;
  rb->max_elts = params->max_elts;

  ER_DEBUG("type=%s,max_elts=%zu,elt_size=%zu,flags=0x%08x",
           RCSW_DS_RBUFFER_TYPE(rb),
           rb->max_elts,
           rb->elt_size,
           rb->flags);
  return rb;

error:
  rbuffer_destroy(rb);
  return NULL;
} /* rbuffer_init() */

void rbuffer_destroy(struct rbuffer* rb) {
  RCSW_FPC_V(NULL != rb);

  rcsw_free(rb->elements, rb->flags & RCSW_NOALLOC_DATA);
  rcsw_free(rb, rb->flags & RCSW_NOALLOC_HANDLE);
} /* rbuffer_destroy() */

status_t rbuffer_add(struct rbuffer* const rb, const void* const e) {
  RCSW_FPC_NV(ERROR, rb != NULL, e != NULL);

  /* do not add if acting as FIFO and currently full */
  if ((rb->flags & RCSW_DS_RBUFFER_AS_FIFO) && rbuffer_isfull(rb)) {
    ER_WARN("Not adding new element: FIFO full");
    errno = ENOSPC;
    return ERROR;
  }

  /* add element */
  ds_elt_copy(rbuffer_data_get(rb, (rb->start + rb->current) % rb->max_elts),
              e,
              rb->elt_size);

  /* start wrapped around to end--overwrite */
  if (rbuffer_isfull(rb)) {
    rb->start = (rb->start + 1) % rb->max_elts;
  } else {
    ++rb->current;
  }

  return OK;
} /* rbuffer_add() */

void* rbuffer_data_get(const struct rbuffer* const rb, size_t idx) {
  RCSW_FPC_NV(NULL, rb != NULL);

  idx = idx % rb->max_elts;
  return (uint8_t*)rb->elements + (idx * rb->elt_size);
} /* rbuffer_data_get() */

status_t rbuffer_serve_front(const struct rbuffer* const rb, void* const e) {
  RCSW_FPC_NV(ERROR, rb != NULL, e != NULL, !rbuffer_isempty(rb));
  ds_elt_copy(e, rbuffer_data_get(rb, rb->start), rb->elt_size);
  return OK;
} /* rbuffer_serve_front() */

void* rbuffer_front(const struct rbuffer* const rb) {
  RCSW_FPC_NV(NULL, rb != NULL, !rbuffer_isempty(rb));
  return rbuffer_data_get(rb, rb->start);
} /* rbuffer_serve_front() */

status_t rbuffer_remove(struct rbuffer* const rb, void* const e) {
  RCSW_FPC_NV(ERROR, rb != NULL, !rbuffer_isempty(rb));

  if (e != NULL) {
    rbuffer_serve_front(rb, e);
  }

  rb->start = (rb->start + 1) % rb->max_elts;
  --rb->current;

  return OK;
} /* rbuffer_remove() */

int rbuffer_index_query(struct rbuffer* const rb, const void* const e) {
  RCSW_FPC_NV(ERROR, rb != NULL, rb->cmpe != NULL, e != NULL);

  size_t wrap = 0;
  size_t i = rb->start;
  int rval = ERROR;

  while (!wrap || (i != rb->start)) {
    if (rb->cmpe(e, rbuffer_data_get(rb, i)) == 0) {
      rval = (int)i;
      break;
    }
    /* wrapped around to index 0 */
    if (i + 1 == rb->max_elts) {
      wrap = 1;
      i = 0;
    } else {
      i++;
    }
  } /* while() */

  return rval;
} /* rbuffer_index_query() */

status_t rbuffer_clear(struct rbuffer* const rb) {
  RCSW_FPC_NV(ERROR, rb != NULL);

  memset(rb->elements, 0, rb->current * rb->elt_size);
  rb->current = 0;
  rb->start = 0;

  return OK;
} /* rbuffer_clear() */

status_t rbuffer_map(struct rbuffer* const rb, void (*f)(void* e)) {
  RCSW_FPC_NV(ERROR, rb != NULL, f != NULL);

  size_t count = 0;
  while (count < rb->current) {
    f(rbuffer_data_get(rb, (rb->start + count++) % rb->max_elts));
  } /* for() */

  return OK;
} /* rbuffer_map() */

status_t rbuffer_inject(struct rbuffer* const rb,
                        void (*f)(void* elt, void* res),
                        void* result) {
  RCSW_FPC_NV(ERROR, rb != NULL, f != NULL, result != NULL);

  size_t count = 0;
  while (count < rb->current) {
    f(rbuffer_data_get(rb, (rb->start + count++) % rb->max_elts), result);
  }

  return OK;
} /* rbuffer_inject() */

void rbuffer_print(struct rbuffer* const rb) {
  if (rb == NULL) {
    DPRINTF(RCSW_ER_MODNAME " : < NULL >\n");
    return;
  }
  if (rbuffer_isempty(rb)) {
    DPRINTF(RCSW_ER_MODNAME " : < Empty >\n");
    return;
  }
  if (rb->printe == NULL) {
    DPRINTF(RCSW_ER_MODNAME " : < No print function >\n");
    return;
  }

  for (size_t i = 0; i < rb->current; ++i) {
    rb->printe(rbuffer_data_get(rb, (rb->start + i) % rb->max_elts));
  } /* for() */
  DPRINTF("\n");
} /* rbuffer_print() */

END_C_DECLS
