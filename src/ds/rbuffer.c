/**
 * \file
 *
 * \copyright 2017 John Harwell
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/rbuffer.h"

#include <string.h>

#define RCSW_ER_MODID LOG4CL_DS_RBUFFER
#define RCSW_ER_MODNAME "rcsw.ds.rbuffer"
#include "rcsw/core/alloc.h"
#include "rcsw/ds/ds.h"
#include "rcsw/ds/iter.h"
#include "rcsw/er/client.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/
#define RCSW_DS_RBUFFER_TYPE(rb) \
  (((rb)->flags & RCSW_DS_RBUFFER_AS_FIFO) ? "FIFO" : "RBUFFER")

/*******************************************************************************
 * Private API
 ******************************************************************************/
/*
 * The element in physical slot \p slot (0 <= slot < max_elts), regardless of
 * whether it holds live data.
 */
static void* rbuffer_slot(const struct rbuffer* const rb, size_t slot) {
  return (uint8_t*)rb->elements + (slot * rb->elt_size);
}

/*
 * cursor stores the logical offset from rb->start (i.e. how many elements have
 * been consumed), cast to void*. This avoids storing a signed index and keeps
 * the wrap-around arithmetic in one place.
 */
static void* rbuffer_iter_next_impl(struct ds_iterator* iter) {
  struct rbuffer* rb  = iter->container;
  size_t          off = iter->cursor.idx;

  if (off >= rb->current) {
    return NULL;
  }
  iter->cursor.idx = off + 1;
  return rbuffer_data_get(rb, off);
} /* rbuffer_iter_next_impl() */

const struct ds_ops rbuffer_iter_ops = {
  .next = rbuffer_iter_next_impl,
  .prev = NULL, /* ringbuffer iteration is forward-only */
};

/*******************************************************************************
 * Public API
 ******************************************************************************/
BEGIN_C_DECLS

struct rbuffer* rbuffer_init(struct rbuffer*                    rb_in,
                             const struct rbuffer_config* const params) {
  RCSW_FPC_NV(NULL, params != NULL, params->max_elts > 0, params->elt_size > 0);
  RCSW_ER_MODULE_INIT();

  struct rbuffer* rb =
    rcsw_alloc(rb_in,
               sizeof(struct rbuffer),
               params->flags & (RCSW_NOALLOC_HANDLE | RCSW_ZALLOC));
  RCSW_CHECK_PTR(rb);
  rb->flags = params->flags;

  rb->elements = rcsw_alloc(params->elements,
                            params->max_elts * params->elt_size,
                            params->flags & (RCSW_NOALLOC_DATA | RCSW_ZALLOC));

  RCSW_CHECK_PTR(rb->elements);

  rb->elt_size = params->elt_size;
  rb->printe   = params->printe;
  rb->cmpe     = params->cmpe;
  rb->start    = 0;
  rb->current  = 0;
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
  ds_elt_copy(rbuffer_slot(rb, (rb->start + rb->current) % rb->max_elts),
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
  RCSW_FPC_NV(NULL, rb != NULL, idx < rb->current);

  /* idx is logical: 0 is the front (oldest) element */
  return rbuffer_slot(rb, (rb->start + idx) % rb->max_elts);
} /* rbuffer_data_get() */

status_t rbuffer_serve_front(const struct rbuffer* const rb, void* const e) {
  RCSW_FPC_NV(ERROR, rb != NULL, e != NULL, !rbuffer_isempty(rb));
  ds_elt_copy(e, rbuffer_slot(rb, rb->start), rb->elt_size);
  return OK;
} /* rbuffer_serve_front() */

void* rbuffer_front(const struct rbuffer* const rb) {
  RCSW_FPC_NV(NULL, rb != NULL, !rbuffer_isempty(rb));
  return rbuffer_slot(rb, rb->start);
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
  RCSW_FPC_NV(-1, rb != NULL, e != NULL);
  ER_ASSERT(NULL != rb->cmpe, "rbuffer_index_query() requires cmpe()");

  /* Search live elements from the front; the result is a logical index, as
   * accepted by rbuffer_data_get(). */
  for (size_t off = 0; off < rb->current; ++off) {
    if (rb->cmpe(e, rbuffer_data_get(rb, off)) == 0) {
      return (int)off;
    }
  } /* for(off..) */
  return -1;
} /* rbuffer_index_query() */

status_t rbuffer_clear(struct rbuffer* const rb) {
  RCSW_FPC_NV(ERROR, rb != NULL);

  /* Live elements may wrap around the end, so zero the whole buffer */
  memset(rb->elements, 0, rb->max_elts * rb->elt_size);
  rb->current = 0;
  rb->start   = 0;

  return OK;
} /* rbuffer_clear() */

status_t rbuffer_map(struct rbuffer* const rb, void (*f)(void* e)) {
  RCSW_FPC_NV(ERROR, rb != NULL, f != NULL);

  size_t count = 0;
  while (count < rb->current) {
    f(rbuffer_data_get(rb, count++));
  } /* for() */

  return OK;
} /* rbuffer_map() */

status_t rbuffer_inject(struct rbuffer* const rb,
                        void (*f)(void* elt, void* res),
                        void* result) {
  RCSW_FPC_NV(ERROR, rb != NULL, f != NULL, result != NULL);

  size_t count = 0;
  while (count < rb->current) {
    f(rbuffer_data_get(rb, count++), result);
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
  ER_ASSERT(NULL != rb->printe, "rbuffer_print() requires printe()");

  for (size_t i = 0; i < rb->current; ++i) {
    rb->printe(rbuffer_data_get(rb, i));
  } /* for() */
  DPRINTF("\n");
} /* rbuffer_print() */

struct ds_iterator* rbuffer_iter_init(struct ds_iterator* iter,
                                      struct rbuffer*     rb,
                                      bool_t (*classify)(void* e)) {
  RCSW_FPC_NV(NULL, iter != NULL, rb != NULL);

  iter->cursor.idx = 0; /* start at logical offset 0 */
  return ds_iter_init(iter, rb, ITER_FORWARD, &rbuffer_iter_ops, classify);
} /* rbuffer_iter_init() */

END_C_DECLS
