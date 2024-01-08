/**
 * \file multififo.h
 *
 * \copyright 2024 John Harwell, All rights reserved.
 *
 * SPDX-License Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/fifo.h"
#include "rcsw/common/fpc.h"

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/
/**
 * \brief Parameters for \ref multififo.
 */
struct multififo_params {
  /**
   * Pointer to application-allocated space for storing data managed by the \ref
   * multififo. Ignored unless \ref RCSW_NOALLOC_DATA is passed.
   */
  dptr_t *elements;

  /**
   * Pointer to application-allocated space for storing child shadow
   * FIFOs. Ignored unless \ref RCSW_NOALLOC_META is passed.
   */
  dptr_t* meta;

  /**
   * Size of the "base" elements (i.e., the elements in the main FIFO) in bytes.
   */
  size_t elt_size;

  /**
   * Maximum number of elements allowed.
   */
  size_t max_elts;

  /**
   * The # of child shadow FIFOs. Should be > 0; if it is 0, why are you using a
   * multi-FIFO?
   */
  size_t n_children;

  /**
   * Configuration for child shadow FIFOs. Currently only the element size is
   * needed; if this changes, either \ref fifo_params or another parameter
   * struct will be used instead.
   */
  size_t* children;

  /**
   * Configuration flags. See \ref multififo.flags for valid flags.
   */
  uint32_t flags;
};

/**
 * Convenience structure for making management of child FIFOs and their data
 * more readable in the code.
 */
struct multififo_children_mgmt {
  /**
   * Child shadow FIFOs containing references to the data in \ref
   * multififo.root. Implictly uses \ref RCSW_NOALLOC_DATA, \ref
   * RCSW_NOALLOC_HANDLE.
   */
  struct fifo* fifos;

  /**
   * Pointer to space for child FIFO elements; each FIFO has (root FIFO elt size
   * / child FIFO elt size) elements in it.
   */

  dptr_t* elements;

  /**
   * # of child FIFOs.
   */
  size_t count;
};

/**
 * \brief A FIFO which itself contains multiple child shadow FIFOs.
 *
 * Useful when you want to put large packets of data into a FIFO, but have
 * consumers pull them out in much smaller chunks, such as when you have a
 * process which produces large packets of data which you want to send over UART
 * without disrupting/preempting the main process. This can be easily done with
 * this data structure and a say 1ms "tick" ISR which pulls data out of a child
 * FIFO as it becomes available.
 *
 * All data lives in the "root" FIFO; no copies are made to child FIFOs.
 *
 * NOT thread safe.
 *
 * Synchronization safe, as in non-concurrent asynchronous accesses are OK: FIFO
 * provides a simple internal boolean "lock" which is taken and release during:
 *
 * - \ref multififo_add()
 * - \ref multififo_remove()
 *
 * All other functions must be called synchronously or bad things will probably
 * happen.
 */
struct multififo {
  /**
   * Root FIFO. All actual data is stored here.
   */
  struct fifo root;

  /**
   * Bitmask indicating which child FIFOs have finished processing the current
   * front element in \ref multififo.root, and which haven't. Child FIFO \c i
   * sets bit \code 1 << i \endcode.
   */

  uint8_t front_refmask;

  /**
   * Super simple, non-threadsafe synchronization mechanism indicating if the
   * \ref multififo is currently busy doing stuff. Callers consuming data
   * through child FIFOs should check this flag before attempting to remove any
   * data; if it is set, then an operation is in progress and any sort of
   * dequeueing will probably cause errors.
   */
  bool locked;

  /**
   * Management of all root and child datablocks and metadata for the multi-FIFO.
   */
  struct multififo_children_mgmt children;

  /**
   * Run-time configuration parameters. Valid flags are:
   *
   * - \ref RCSW_ZALLOC
   * - \ref RCSW_NOALLOC_HANDLE
   * - \ref RCSW_NOALLOC_DATA
   *
   * All other flags are ignored.
   */
  uint32_t flags;
};

BEGIN_C_DECLS

/*******************************************************************************
 * API Functions
 ******************************************************************************/
/**
 * \brief Determine if the multi-FIFO is currently full.
 *
 * \note This works with the \a root FIFO only--to check if a child shadow FIFO
 *       is full, use \ref fifo_isfull() with an appropriate \ref fifo
 *       reference.
 *
 * \param fifo The multi-FIFO handle.
 *
 * \return \ref bool_t
 */
static inline bool_t multififo_isfull(const struct multififo* const fifo) {
    RCSW_FPC_NV(false, NULL != fifo);
    return fifo_isfull(&fifo->root);
}

/**
 * \brief Determine if the multi-FIFO is currently empty
 *
 * \note This works with the \a root FIFO only--to check if a child shadow FIFO
 *       is empty, use \ref fifo_isempty() with an appropriate \ref fifo
 *       reference.
 *
 * \param fifo The multi-FIFO handle.
 *
 * \return \ref bool_t
 */
static inline bool_t multififo_isempty(const struct multififo* const fifo) {
    RCSW_FPC_NV(false, NULL != fifo);
    return fifo_isempty(&fifo->root);
}

/**
 * \brief Determine # elements currently in the multi-FIFO.
 *
 * \note This works with the \a root FIFO only--to get the # elements in a child
 *       shadow FIFO, use \ref fifo_size() with an appropriate \ref fifo
 *       reference.
 *
 * \param fifo The multi-FIFO handle.
 *
 * \return # elements in the multi-FIFO, or 0 on ERROR.
 */

static inline size_t multififo_size(const struct multififo* const fifo) {
    RCSW_FPC_NV(0, NULL != fifo);
    return fifo_size(&fifo->root);
}

/**
 * \brief Determine if the multi-FIFO is currently locked (busy) or is safe for
 * child processes to try to receive data from.
 *
 * \param fifo The multi-FIFO handle.
 *
 * \return # elements in the multi-FIFO, or 0 on ERROR.
 */

static inline size_t multififo_islocked(const struct multififo* const fifo) {
    RCSW_FPC_NV(0, NULL != fifo);
    return fifo->locked;
}

/**
 * \brief Get the multi-FIFO capacity.
 *
 * \note This works with the \a root FIFO only--to get capacity of a child
 *       shadow FIFO, use \ref fifo_capacity() with an appropriate \ref fifo
 *       reference.
 *
 * \param fifo The multi-FIFO handle.
 *
 * \return Capacity of the multi-FIFO, or 0 on ERROR.
 */
static inline size_t multififo_capacity(const struct multififo* const fifo) {
    RCSW_FPC_NV(0, NULL != fifo);
    return fifo_capacity(&fifo->root);
}

/**
 * \brief Calculate the # of bytes that the multi- FIFO will require if \ref
 * RCSW_NOALLOC_DATA is passed to manage a specified # of elements of a
 * specified size.
 *
 * \param max_elts # of desired elements the multi-FIFO will hold.
 *
 * \param elt_size size of elements in the multi-FIFO in bytes.
 *
 * \return The total # of bytes the application would need to allocate
 */
static inline size_t multififo_element_space(size_t max_elts, size_t elt_size) {
    return fifo_element_space(max_elts, elt_size);
}

/**
 * \brief Calculate the # of bytes needed for all child shadow FIFOs to point
 * into elements in \ref multififo.root .
 *
 * \param elt_size Sizeof root FIFO elements in bytes.
 *
 * \param n_children # of shadow child FIFOs.
 *
 * \return The total # of bytes the application would need to allocate.
 */
static inline size_t multififo_meta_space(size_t elt_size, size_t n_children) {
  /*
   * Each child FIFO needs a handle + 1 root FIFO element worth of space to
   * store its elements.
   */
  return (sizeof(struct fifo) + elt_size) * n_children;
}

/**
 * \brief Initialize a multi-FIFO.
 *
 * \param fifo_in An application allocated handle for the multi-FIFO. Cannot be
 *                NULL if \ref RCSW_NOALLOC_HANDLE is passed in \ref
 *                multififo_params.flags.
 *
 * \param params The initialization parameters.
 *
 * \return The initialized multi-FIFO, or NULL if an error occurred.
 */
RCSW_API struct multififo *multififo_init(struct multififo *fifo_in,
                                            const struct multififo_params * params) RCSW_WUR;

/**
 * \brief Destroy a multi-FIFO.
 *
 * Any further use of the multi-FIFO after this function is called is undefined.
 *
 * \param fifo The multi-FIFO to destroy.
 */
RCSW_API void multififo_destroy(struct multififo *fifo);

/**
 * \brief Enqueue an element into the multi-FIFO.
 *
 * 1. Copy the element into an available slot in the root FIFO.
 *
 * 2. If the ref count for the front/head element in the root FIFO is 0, then
 *    add an appropriate number of elements to each child shadow FIFO pointing
 *    inside the front element in the root FIFO (which might not be the one just
 *    added).
 *
 * \param fifo The multi-FIFO handle.
 *
 * \param e The element to enqueue. Cannot be NULL.
 *
 * \return \ref status_t .
 */
RCSW_API status_t multififo_add(struct multififo * fifo, const void * e);

/**
 * \brief Get the first multi-FIFO item without removing it.
 *
 * \param fifo The multi-FIFO handle.
 *
 * \return Pointer to the first element, or NULL if no such element or an error
 * occurred.
 */
static inline void* multififo_front(const struct multififo* const fifo) {
    RCSW_FPC_NV(0, NULL != fifo);
    return fifo_front(&fifo->root);
}

/**
 * \brief Dequeue an element from the multi-FIFO.
 *
 * If the ref count for the front/head element in the root FIFO is 0, then the
 * element will be dequeued into \p e (assuming it is non-NULL). Otherwise, no
 * effect. If a remove is performed, an appropriate number of elements are added
 * to each child shadow FIFO pointing inside the new front element in the root
 * FIFO.
 *
 * \param fifo The multi-FIFO handle.
 *
 * \param e The element to dequeue into from the multi-FIFO. Can be NULL.
 *
 * \return \ref status_t . Note that it is an ERROR to try to remove an element
 * from the root FIFO before all children have finished processing it.
 */
RCSW_API status_t multififo_remove(struct multififo * fifo, void * e);

/**
 * \brief Clear a multi-FIFO.
 *
 * Empty the FIFO, but do not deallocate its memory. This also clears all child
 * shadow FIFOs.
 *
 * \return \ref status_t.
 */
RCSW_API status_t multififo_clear(struct multififo * fifo);

END_C_DECLS
