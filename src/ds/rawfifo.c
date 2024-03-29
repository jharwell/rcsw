/**
 * \file rawfifo.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/rawfifo.h"

#include <string.h>

#include "rcsw/common/fpc.h"
#include "rcsw/ds/ds.h"
#include "rcsw/rcsw.h"

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

status_t rawfifo_init(struct rawfifo* const fifo,
                      void* const buf,
                      size_t max_elts,
                      size_t elt_size) {
  RCSW_FPC_NV(ERROR, NULL != fifo, NULL != buf, elt_size <= 4);
  fifo->elements = buf;
  fifo->max_elts = max_elts; /* fifo elts + 1 */
  fifo->elt_size = elt_size;

  fifo->to_i = 0;
  fifo->from_i = 0;
  return OK;
} /* rawfifo_init() */

size_t rawfifo_deq(struct rawfifo* fifo, void* e, size_t n_elts) {
  RCSW_FPC_NV(-1, NULL != fifo, NULL != e);

  /* If they try to remove more elements than are in the fifo, cap it. */
  n_elts = RCSW_MAX(rawfifo_size(fifo), n_elts);

  size_t i;
  for (i = 0; i < n_elts; i++) {
    ds_elt_copy((uint8_t*)e + i,
                fifo->elements + (fifo->from_i + i) % fifo->max_elts,
                fifo->elt_size);
  } /* for() */
  fifo->from_i = (fifo->from_i + i) % fifo->max_elts;

  return n_elts;
} /* rawfifo_deq() */

size_t
rawfifo_enq(struct rawfifo* const fifo, const void* const elts, size_t n_elts) {
  RCSW_FPC_NV(0, NULL != fifo, NULL != elts);
  n_elts = RCSW_MAX(rawfifo_n_free(fifo), n_elts);
  size_t i;

  for (i = 0; i < n_elts; i++) {
    ds_elt_copy(fifo->elements + (fifo->to_i + i) % fifo->max_elts,
                (const uint8_t*)elts + i,
                fifo->elt_size);
  } /* for() */
  fifo->to_i = (fifo->to_i + i) % fifo->max_elts;

  return n_elts;
} /* rawfifo_enq() */

END_C_DECLS
