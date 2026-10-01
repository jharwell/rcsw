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
#include "rcsw/ds/rawfifo.h"

#include <string.h>

#include "rcsw/core/compilers.h"
#include "rcsw/core/fpc.h"
#include "rcsw/ds/ds.h"

/*******************************************************************************
 * Public API
 ******************************************************************************/
BEGIN_C_DECLS

/* Address of slot i: elements are packed at an elt_size-byte stride */
#define RAWFIFO_SLOT(fifo, i) \
  ((uint8_t*)(fifo)->elements + (((i) % (fifo)->max_elts) * (fifo)->elt_size))

status_t rawfifo_init(struct rawfifo* const fifo,
                      void* const           buf,
                      size_t                max_elts,
                      size_t                elt_size) {
  RCSW_FPC_NV(ERROR,
              NULL != fifo,
              NULL != buf,
              1 == elt_size || 2 == elt_size || 4 == elt_size,
              max_elts >= 2);
  fifo->elements = buf;
  fifo->max_elts = max_elts; /* fifo elts + 1 */
  fifo->elt_size = elt_size;

  atomic_init(&fifo->to_i, 0);
  atomic_init(&fifo->from_i, 0);
  return OK;
} /* rawfifo_init() */

size_t rawfifo_deq(struct rawfifo* fifo, void* e, size_t n_elts) {
  RCSW_FPC_NV(0, NULL != fifo, NULL != e);

  /*
   * Consumer side: from_i is ours, to_i is the producer's. The acquire load of
   * to_i makes the producer's element writes visible before we read them.
   */
  size_t from  = atomic_load_explicit(&fifo->from_i, memory_order_relaxed);
  size_t to    = atomic_load_explicit(&fifo->to_i, memory_order_acquire);
  size_t avail = (to >= from) ? (to - from) : (to + (fifo->max_elts - from));

  /* If they try to remove more elements than are in the fifo, cap it. */
  n_elts = RCSW_MIN(avail, n_elts);

  for (size_t i = 0; i < n_elts; i++) {
    memcpy((uint8_t*)e + (i * fifo->elt_size),
           RAWFIFO_SLOT(fifo, from + i),
           fifo->elt_size);
  } /* for() */

  /* release: our reads of the slots complete before the producer reuses them */
  atomic_store_explicit(&fifo->from_i,
                        (from + n_elts) % fifo->max_elts,
                        memory_order_release);
  return n_elts;
} /* rawfifo_deq() */

size_t rawfifo_enq(struct rawfifo* const fifo,
                   const void* const     elts,
                   size_t                n_elts) {
  RCSW_FPC_NV(0, NULL != fifo, NULL != elts);

  /*
   * Producer side: to_i is ours, from_i is the consumer's. The acquire load of
   * from_i guarantees the consumer is done reading any slot we overwrite.
   */
  size_t to   = atomic_load_explicit(&fifo->to_i, memory_order_relaxed);
  size_t from = atomic_load_explicit(&fifo->from_i, memory_order_acquire);
  size_t used = (to >= from) ? (to - from) : (to + (fifo->max_elts - from));

  /* One slot is always left empty so that full and empty are distinguishable */
  n_elts = RCSW_MIN(fifo->max_elts - used - 1, n_elts);

  for (size_t i = 0; i < n_elts; i++) {
    memcpy(RAWFIFO_SLOT(fifo, to + i),
           (const uint8_t*)elts + (i * fifo->elt_size),
           fifo->elt_size);
  } /* for() */

  /* release: element writes are visible before the consumer sees new to_i */
  atomic_store_explicit(&fifo->to_i,
                        (to + n_elts) % fifo->max_elts,
                        memory_order_release);
  return n_elts;
} /* rawfifo_enq() */

END_C_DECLS
