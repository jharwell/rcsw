/**
 * \file multififo.c
 *
 * \copyright 2024 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/multififo.h"

#define RCSW_ER_MODNAME RCSW_ER_MODNAME_BUILDER("rcsw", "ds", "multififo")
#define RCSW_ER_MODID ekLOG4CL_DS_MULTIFIFO
#include "rcsw/er/client.h"
#include "rcsw/common/alloc.h"

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
    size_t n_elts = fifo->root.elt_size / fifo->children.fifos[i].elt_size;

    for (size_t j = 0; j < n_elts; ++j) {
      uint8_t* target = (uint8_t*)fifo_front(&fifo->root) + fifo->children.fifos[i].elt_size * j;
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
      fifo->front_refmask &= ~(1 << i);
    }
  } /* for(i..) */
} /* multififo_children_status_update() */

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

struct multififo* multififo_init(struct multififo* fifo_in,
                                   const struct multififo_params* const params) {
  RCSW_FPC_NV(NULL,
              params != NULL,
              params->max_elts > 0,
              params->elt_size > 0);
  RCSW_ER_MODULE_INIT();

  struct multififo* fifo = rcsw_alloc(fifo_in,
                                 sizeof(struct multififo),
                                 params->flags & RCSW_NOALLOC_HANDLE);

  RCSW_CHECK_PTR(fifo);
  fifo->flags = params->flags;
  fifo->locked = false;
  fifo->children.count = params->n_children;
  fifo->front_refmask = 0;

  struct fifo_params root_params = {
    .printe = NULL,
    .elt_size = params->elt_size,
    .max_elts = params->max_elts,
    .elements = params->elements,
    .flags = params->flags
  };
  root_params.flags |= RCSW_NOALLOC_HANDLE;
  RCSW_CHECK(NULL != fifo_init(&fifo->root, &root_params));

  fifo->children.fifos = rcsw_alloc(params->meta,
                                    sizeof(struct fifo) * params->n_children,
                                    params->flags & RCSW_NOALLOC_META);
  RCSW_CHECK_PTR(fifo->children.fifos);
  fifo->children.elements = (dptr_t*)((uint8_t*)params->meta +
                                   sizeof(struct fifo) * params->n_children);

  for (size_t i = 0; i < fifo->children.count; ++i) {
    ER_CHECK(0 == params->elt_size % params->children[i],
             "Child FIFO %zu size=%zu not a multiple of root FIFO elt_size=%zu",
             i,
             params->children[i],
             params->elt_size);
    size_t max_elts = params->elt_size / params->children[i];
    ER_DEBUG("Child FIFO %zu: %zu elts", i, max_elts);
    struct fifo_params child_params = {
      .printe = NULL,
      .elt_size = params->children[i],
      .max_elts = max_elts,
      .elements = (dptr_t*)((uint8_t*)fifo->children.elements + params->elt_size * i),
      .flags = params->flags | RCSW_NOALLOC_HANDLE
    };
    RCSW_CHECK(NULL != fifo_init(&fifo->children.fifos[i], &child_params));
  } /* for(i..) */

  return fifo;

error:
  multififo_destroy(fifo);
  errno = EAGAIN;
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

  RCSW_CHECK(!fifo->locked);
  fifo->locked = true;

  bool ret = fifo_add(&fifo->root, e);
  RCSW_CHECK(OK == ret);

  multififo_children_status_update(fifo);
  /*
   * All children have finished processing root FIFO front element--onto the
   * next one.
   */
  if (0 == fifo->front_refmask) {
    RCSW_CHECK(OK == multififo_children_feed(fifo));
  }
  fifo->locked = false;
  return OK;

error:
  fifo->locked = false;
  return ERROR;
} /* multififo_add() */

status_t multififo_remove(struct multififo* const fifo, void* const e) {
  RCSW_FPC_NV(ERROR, NULL != fifo);

  RCSW_CHECK(!fifo->locked);
  fifo->locked = true;

  multififo_children_status_update(fifo);

  /*
   * All children have finished processing root FIFO front element--OK to remove
   * it and update child FIFOs to point into the new front element.
   */
  RCSW_CHECK(0 == fifo->front_refmask);
  bool ret =  fifo_remove(&fifo->root, e);
  RCSW_CHECK(OK == ret);
  RCSW_CHECK(OK == multififo_children_feed(fifo));

  fifo->locked = false;
  return OK;

error:
  fifo->locked = false;
  return ERROR;
} /* multififo_remove() */

status_t multififo_clear(struct multififo* const fifo) {
  RCSW_FPC_NV(ERROR, NULL != fifo);

  fifo_clear(&fifo->root);
  for (size_t i = 0; i < fifo->children.count; ++i) {
    fifo_clear(&fifo->children.fifos[i]);
  } /* for(i..) */
  fifo->front_refmask = 0;
  fifo->locked = false;

  return OK;
} /* multififo_clear() */

END_C_DECLS
