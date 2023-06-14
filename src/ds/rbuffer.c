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

#include "rcsw/common/dbg.h"
#include "rcsw/ds/ds.h"

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
#define MODULE_ID M_DS_RBUFFER

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
                             const struct ds_params* const params) {
  RCSW_FPC_NV(NULL,
              params != NULL,
              params->tag == DS_RBUFFER,
              params->max_elts > 0,
              params->elt_size > 0);

  struct rbuffer* rb = NULL;
  if (params->flags & RCSW_DS_NOALLOC_HANDLE) {
    RCSW_CHECK_PTR(rb_in);
    rb = rb_in;
  } else {
    rb = malloc(sizeof(struct rbuffer));
    RCSW_CHECK_PTR(rb);
  }
  rb->flags = params->flags;

  if (params->flags & RCSW_DS_NOALLOC_DATA) {
    RCSW_CHECK_PTR(params->elements);
    rb->elements = params->elements;
  } else {
    rb->elements = calloc(params->max_elts, params->elt_size);
    RCSW_CHECK_PTR(rb->elements);
  }

  rb->elt_size = params->elt_size;
  rb->printe = params->printe;
  rb->cmpe = params->cmpe;
  rb->start = 0;
  rb->current = 0;
  rb->max_elts = params->max_elts;

  DBGD("type: %s max_elts=%zu elt_size=%zu flags=0x%08x\n",
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

  if (!(rb->flags & RCSW_DS_NOALLOC_DATA)) {
    if (rb->elements) {
      free(rb->elements);
      rb->elements = NULL;
    }
  }

  if (!(rb->flags & RCSW_DS_NOALLOC_HANDLE)) {
    free(rb);
  }
} /* rbuffer_destroy() */

status_t rbuffer_add(struct rbuffer* const rb, const void* const e) {
  RCSW_FPC_NV(ERROR, rb != NULL, e != NULL);

  /* do not add if acting as FIFO and currently full */
  if ((rb->flags & RCSW_DS_RBUFFER_AS_FIFO) && rbuffer_isfull(rb)) {
    DBGW("WARNING: Not adding new element: FIFO full\n");
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

void* rbuffer_data_get(const struct rbuffer* const rb, size_t key) {
  RCSW_FPC_NV(NULL, rb != NULL, key < rb->max_elts);

  return rb->elements + (key * rb->elt_size);
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
    DPRINTF("Ringbuffer: < NULL ringbuffer >\n");
    return;
  }
  if (rbuffer_isempty(rb)) {
    DPRINTF("Ringbuffer: < Empty Ringbuffer >\n");
    return;
  }
  if (rb->printe == NULL) {
    DPRINTF("Ringbuffer: < No print function >\n");
    return;
  }

  size_t i;
  for (i = 0; i < rb->current; ++i) {
    rb->printe(rbuffer_data_get(rb, (rb->start + i) % rb->max_elts));
  } /* for() */
  DPRINTF("\n");
} /* rbuffer_print() */

END_C_DECLS
