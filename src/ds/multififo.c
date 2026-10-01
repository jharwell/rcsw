/**
 * \file
 *
 * \copyright 2024 John Harwell
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/multififo.h"

#include <errno.h>
#include <limits.h>

#define RCSW_ER_MODNAME RCSW_ER_MODNAME_BUILDER("rcsw", "ds", "multififo")
#define RCSW_ER_MODID LOG4CL_DS_MULTIFIFO
#include "rcsw/core/alloc.h"
#include "rcsw/er/client.h"

/*******************************************************************************
 * Private Functions
 ******************************************************************************/
static status_t multififo_children_feed(struct multififo* fifo) {
  /* No food available! */
  if (multififo_isempty(fifo)) {
    return OK;
  }
  fifo->front_refmask = 0;
  for (size_t i = 0; i < fifo->children.count; ++i) {
    /*
     * Each child FIFO may/will have a different # elements in it, depending on
     * its chunk size vs. the root FIFO.
     */
    size_t n_elts = fifo->root.rb.elt_size / fifo->children.fifos[i].rb.elt_size;

    for (size_t j = 0; j < n_elts; ++j) {
      uint8_t* target = (uint8_t*)fifo_front(&fifo->root) +
                        (fifo->children.fifos[i].rb.elt_size * j);
      RCSW_CHECK(OK == fifo_add(&fifo->children.fifos[i], target));
      fifo->front_refmask |= 1 << i;
    } /* for(j..) */
  } /* for(i..) */

  return OK;

error:
  return ERROR;
}

static void multififo_children_status_update(struct multififo* fifo) {
  for (size_t i = 0; i < fifo->children.count; ++i) {
    if (fifo_isempty(&fifo->children.fifos[i])) {
      fifo->front_refmask &= (uint8_t)(~(1UL << i));
    }
  } /* for(i..) */
} /* multififo_children_status_update() */

/*******************************************************************************
 * Public API
 ******************************************************************************/
BEGIN_C_DECLS
/* NOLINTNEXTLINE(readability-function-size) */
struct multififo* multififo_init(struct multififo*                    fifo_in,
                                 const struct multififo_config* const params) {
  RCSW_FPC_NV(NULL,
              params != NULL,
              params->max_elts > 0,
              params->elt_size > 0,
              params->n_children > 0,
              params->children != NULL);
  RCSW_ER_MODULE_INIT();

  struct multififo* fifo = NULL;

  /* front_refmask has one bit per child */
  ER_CHECK(params->n_children <= sizeof(fifo->front_refmask) * CHAR_BIT,
           "Too many children: %zu > %zu",
           params->n_children,
           sizeof(fifo->front_refmask) * CHAR_BIT);
  for (size_t i = 0; i < params->n_children; ++i) {
    ER_CHECK(
      params->children[i] > 0 && 0 == params->elt_size % params->children[i],
      "Child FIFO %zu size=%zu not a divisor of root FIFO elt_size=%zu",
      i,
      params->children[i],
      params->elt_size);
  } /* for(i..) */
  if (params->flags & RCSW_NOALLOC_META) {
    ER_CHECK(NULL != params->meta, "RCSW_NOALLOC_META requires meta space");
  }

  fifo = rcsw_alloc(fifo_in,
                    sizeof(struct multififo),
                    params->flags & (RCSW_NOALLOC_HANDLE | RCSW_ZALLOC));
  if (NULL == fifo) {
    errno = ENOMEM;
    return NULL;
  }
  fifo->flags = params->flags;
  atomic_init(&fifo->locked, false);
  fifo->children.count = 0; /* incremented as children are initialized */
  fifo->children.fifos = NULL;
  fifo->front_refmask  = 0;

  struct fifo_config root_params = {.printe   = NULL,
                                    .elt_size = params->elt_size,
                                    .max_elts = params->max_elts,
                                    .elements = params->elements,
                                    .flags    = params->flags};
  root_params.flags |= RCSW_NOALLOC_HANDLE;
  RCSW_CHECK(NULL != fifo_init(&fifo->root, &root_params));

  /*
   * Child FIFO handles and their element space live in one block: the
   * caller's meta space, or a single heap allocation of the same size.
   */
  fifo->children.fifos =
    rcsw_alloc(params->meta,
               multififo_meta_space(params->elt_size, params->n_children),
               params->flags & (RCSW_NOALLOC_META | RCSW_ZALLOC));
  RCSW_CHECK_PTR(fifo->children.fifos);
  fifo->children.elements = (dptr_t*)((uint8_t*)fifo->children.fifos +
                                      (sizeof(struct fifo) * params->n_children));

  for (size_t i = 0; i < params->n_children; ++i) {
    size_t max_elts = params->elt_size / params->children[i];
    ER_DEBUG("Child FIFO %zu: %zu elts", i, max_elts);
    struct fifo_config child_params = {
      .printe   = NULL,
      .elt_size = params->children[i],
      .max_elts = max_elts,
      .elements =
        (dptr_t*)((uint8_t*)fifo->children.elements + (params->elt_size * i)),
      /* children always use space from the block above */
      .flags = RCSW_NOALLOC_HANDLE | RCSW_NOALLOC_DATA};
    RCSW_CHECK(NULL != fifo_init(&fifo->children.fifos[i], &child_params));
    fifo->children.count++;
  } /* for(i..) */

  return fifo;

error:
  if (NULL != fifo) {
    multififo_destroy(fifo);
    errno = ENOMEM;
  } else {
    errno = EINVAL;
  }
  return NULL;
} /* multififo_init() */

void multififo_destroy(struct multififo* const fifo) {
  RCSW_FPC_V(NULL != fifo);

  fifo_destroy(&fifo->root);
  if (NULL != fifo->children.fifos) {
    for (size_t i = 0; i < fifo->children.count; ++i) {
      fifo_destroy(&fifo->children.fifos[i]);
    } /* for(i..) */
  }
  rcsw_free(fifo->children.fifos, fifo->flags & RCSW_NOALLOC_META);
  rcsw_free(fifo, fifo->flags & RCSW_NOALLOC_HANDLE);
} /* multififo_destroy() */

status_t multififo_add(struct multififo* const fifo, const void* const e) {
  RCSW_FPC_NV(ERROR, NULL != fifo, NULL != e);

  if (atomic_exchange_explicit(&fifo->locked, true, memory_order_acquire)) {
    errno = EAGAIN; /* busy; the lock belongs to someone else */
    return ERROR;
  }

  RCSW_CHECK(OK == fifo_add(&fifo->root, e));

  multififo_children_status_update(fifo);
  /*
   * All children have finished processing root FIFO front element--onto the
   * next one.
   */
  if (0 == fifo->front_refmask) {
    RCSW_CHECK(OK == multififo_children_feed(fifo));
  }
  atomic_store_explicit(&fifo->locked, false, memory_order_release);
  return OK;

error:
  atomic_store_explicit(&fifo->locked, false, memory_order_release);
  return ERROR;
} /* multififo_add() */

status_t multififo_remove(struct multififo* const fifo, void* const e) {
  RCSW_FPC_NV(ERROR, NULL != fifo);

  if (atomic_exchange_explicit(&fifo->locked, true, memory_order_acquire)) {
    errno = EAGAIN; /* busy; the lock belongs to someone else */
    return ERROR;
  }

  multififo_children_status_update(fifo);

  /*
   * All children have finished processing root FIFO front element--OK to remove
   * it and update child FIFOs to point into the new front element.
   */
  RCSW_CHECK(0 == fifo->front_refmask);
  RCSW_CHECK(OK == fifo_remove(&fifo->root, e));
  RCSW_CHECK(OK == multififo_children_feed(fifo));

  atomic_store_explicit(&fifo->locked, false, memory_order_release);
  return OK;

error:
  atomic_store_explicit(&fifo->locked, false, memory_order_release);
  return ERROR;
} /* multififo_remove() */

status_t multififo_clear(struct multififo* const fifo) {
  RCSW_FPC_NV(ERROR, NULL != fifo);

  fifo_clear(&fifo->root);
  for (size_t i = 0; i < fifo->children.count; ++i) {
    fifo_clear(&fifo->children.fifos[i]);
  } /* for(i..) */
  fifo->front_refmask = 0;
  atomic_store_explicit(&fifo->locked, false, memory_order_release);

  return OK;
} /* multififo_clear() */

END_C_DECLS
