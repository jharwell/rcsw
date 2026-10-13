/**
 * \file
 *
 * \copyright 2017 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * \ingroup ds
 *
 * \brief General-purpose FIFO built on a ringbuffer.
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/al/types.h"
#include "rcsw/core/compilers.h"
#include "rcsw/core/core.h"
#include "rcsw/core/fpc.h"
#include "rcsw/ds/ds.h"
#include "rcsw/ds/rbuffer.h"

/*******************************************************************************
 * Types
 ******************************************************************************/
/**
 * \brief Parameters for \ref fifo.
 */
struct fifo_config {
  /**
   * For printing an element. Can be NULL. If NULL, you can't use \ref
   * fifo_print().
   */
  void (*printe)(const void* e);

  /**
   * Pointer to application-allocated space for storing data managed by the \ref
   * fifo. Ignored unless \ref RCSW_NOALLOC_DATA is passed.
   */
  dptr_t* elements;

  /**
   * Size of elements in bytes.
   */
  size_t elt_size;

  /**
   * Maximum number of elements allowed.
   */
  size_t max_elts;

  /**
   * Configuration flags. See \ref fifo for valid flags.
   */
  uint32_t flags;
};

/**
 * \brief General purpose FIFO.
 *
 * A thin, type-safe wrapper around a \ref rbuffer created with \ref
 * RCSW_DS_RBUFFER_AS_FIFO; every fifo_*() function is static inline and
 * forwards to the matching rbuffer_*() function. This implementation is more
 * full-featured than \ref rawfifo, but is not safe to use in ISRs.
 *
 * Handle storage and the valid flags are as for \ref rbuffer, namely \ref
 * RCSW_ZALLOC, \ref RCSW_NOALLOC_HANDLE and \ref RCSW_NOALLOC_DATA (in \ref
 * fifo_config.flags); all other flags are ignored.
 */
struct fifo {
  /**
   * Underlying ringbuffer. A \ref fifo is laid out exactly as a \ref rbuffer,
   * so a handle the library allocates is a \ref rbuffer handle.
   */
  struct rbuffer rb;
};

#ifdef __cplusplus
static_assert(sizeof(struct fifo) == sizeof(struct rbuffer),
              "struct fifo must have the same layout as struct rbuffer");
#else
_Static_assert(sizeof(struct fifo) == sizeof(struct rbuffer),
               "struct fifo must have the same layout as struct rbuffer");
#endif

/*******************************************************************************
 * Public API
 ******************************************************************************/
BEGIN_C_DECLS
/**
 * \brief Determine if the FIFO is currently full
 *
 * \param fifo The FIFO handle
 */
static inline bool_t fifo_isfull(const struct fifo* const fifo) {
  RCSW_FPC_NV(false, NULL != fifo);
  return rbuffer_isfull(&fifo->rb);
}

/**
 * \brief Determine if the FIFO is currently empty
 *
 * \param fifo The FIFO handle
 */
static inline bool_t fifo_isempty(const struct fifo* const fifo) {
  RCSW_FPC_NV(false, NULL != fifo);
  return rbuffer_isempty(&fifo->rb);
}

/**
 * \brief Determine # elements currently in the FIFO.
 *
 * \param fifo The FIFO handle.
 *
 * \return # elements in FIFO, or 0 on ERROR.
 */

static inline size_t fifo_size(const struct fifo* const fifo) {
  RCSW_FPC_NV(0, NULL != fifo);
  return rbuffer_size(&fifo->rb);
}

/**
 * \brief Get the FIFO capacity.
 *
 * \param fifo The FIFO handle.
 *
 * \return Capacity of the FIFO, or 0 on ERROR.
 */
static inline size_t fifo_capacity(const struct fifo* const fifo) {
  RCSW_FPC_NV(0, NULL != fifo);
  return rbuffer_capacity(&fifo->rb);
}

/**
 * \brief Get the first FIFO item without removing it.
 *
 * \param fifo The FIFO handle.
 *
 * \return Pointer to the first element, or NULL if no such element or an error
 * occurred.
 */
static inline void* fifo_front(const struct fifo* const fifo) {
  RCSW_FPC_NV(0, NULL != fifo);
  return rbuffer_front(&fifo->rb);
}

/**
 * \brief Calculate the # of bytes that the FIFO will require if \ref
 * RCSW_NOALLOC_DATA is passed to manage a specified # of elements of a
 * specified size
 *
 * \param max_elts # of desired elements the FIFO will hold
 * \param elt_size size of elements in bytes
 *
 * \return The total # of bytes the application would need to allocate
 */
static inline size_t fifo_element_space(size_t max_elts, size_t elt_size) {
  return rbuffer_element_space(max_elts, elt_size);
}

/**
 * \brief Initialize a FIFO.
 *
 * \param fifo_in Caller storage for the handle, used only if \ref
 *                RCSW_NOALLOC_HANDLE is passed; ignored (may be NULL)
 *                otherwise. See \rcswdoc{concepts/memory-model}.
 *
 * \param params The initialization parameters.
 *
 * \return The initialized FIFO, or NULL if an error occurred (errno is set by
 * \ref rbuffer_init()).
 */
static inline RCSW_WUR struct fifo* fifo_init(
  struct fifo* const fifo_in, const struct fifo_config* const params) {
  RCSW_FPC_NV(NULL, NULL != params);
  struct rbuffer_config rb_params = {
    .cmpe     = NULL,
    .printe   = params->printe,
    .elements = params->elements,
    .elt_size = params->elt_size,
    .max_elts = params->max_elts,
    .flags    = params->flags | RCSW_DS_RBUFFER_AS_FIFO};
  /* struct fifo is struct rbuffer's only member, so the pointers convert */
  return (struct fifo*)rbuffer_init((struct rbuffer*)fifo_in, &rb_params);
}

/**
 * \brief Destroy a FIFO.
 *
 * Any further use of the FIFO after this function is called is undefined.
 *
 * \param fifo The FIFO to destroy.
 */
static inline void fifo_destroy(struct fifo* const fifo) {
  RCSW_FPC_V(NULL != fifo);
  rbuffer_destroy(&fifo->rb);
}

/**
 * \brief Enqueue an element into the FIFO.
 *
 * \param fifo The FIFO handle.
 * \param e The element to enqueue. Cannot be NULL.
 *
 * \return \ref status_t. ERROR with errno=ENOSPC if the FIFO is full.
 */
static inline status_t fifo_add(struct fifo* const fifo, const void* const e) {
  RCSW_FPC_NV(ERROR, NULL != fifo);
  return rbuffer_add(&fifo->rb, e);
}

/**
 * \brief Dequeue an element from the FIFO.
 *
 * \param fifo The FIFO handle.
 * \param e The element to dequeue into from the FIFO. Can be NULL.
 *
 * \return \ref status_t.
 */
static inline status_t fifo_remove(struct fifo* const fifo, void* const e) {
  RCSW_FPC_NV(ERROR, NULL != fifo);
  return rbuffer_remove(&fifo->rb, e);
}

/**
 * \brief Clear a FIFO.
 *
 * Empty the FIFO, but do not deallocate its memory.
 *
 * \return \ref status_t.
 */
static inline status_t fifo_clear(struct fifo* const fifo) {
  RCSW_FPC_NV(ERROR, NULL != fifo);
  return rbuffer_clear(&fifo->rb);
}

/**
 * \brief Apply an operation to all elements of the FIFO.
 *
 * \param fifo The FIFO handle.
 *
 * \param f A function pointer which will be called for every element in the
 *          FIFO. This function CAN modify FIFO elements.
 *
 * \return \ref status_t.
 */
static inline status_t fifo_map(struct fifo* const fifo, void (*f)(void* e)) {
  RCSW_FPC_NV(ERROR, NULL != fifo);
  return rbuffer_map(&fifo->rb, f);
}

/**
 * \brief Compute a cumulative result (sum, count, ...) over all elements of a
 * FIFO.
 *
 * \param fifo The FIFO handle.
 * \param f A function point which will be called for every element in the
 *         FIFO. This function CAN modify FIFO elements.
 *
 * \param result The initial value for the cumulative result to be
 *               computed. This will be passed to each invocation of the
 *               callback.
 *
 * \return \ref status_t.
 */
static inline status_t fifo_inject(struct fifo* const fifo,
                                   void (*f)(void* e, void* res),
                                   void* const result) {
  RCSW_FPC_NV(ERROR, NULL != fifo);
  return rbuffer_inject(&fifo->rb, f, result);
}

/**
 * \brief Print a FIFO. A NULL handle prints a placeholder.
 *
 * \param fifo The FIFO handle.
 */
static inline void fifo_print(struct fifo* const fifo) {
  rbuffer_print(NULL == fifo ? NULL : &fifo->rb);
}

END_C_DECLS
