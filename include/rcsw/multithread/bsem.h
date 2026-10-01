/**
 * \file
 *
 * \copyright 2017 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * \ingroup multithread
 *
 * \brief Binary semaphore.
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <time.h>

#include "rcsw/al/types.h"
#include "rcsw/core/compilers.h"
#include "rcsw/core/core.h"
#include "rcsw/multithread/condv.h"
#include "rcsw/multithread/mutex.h"

/*******************************************************************************
 * Type Definitions
 ******************************************************************************/
/**
 * \brief Wrapper around binary semaphores from various implementations
 *
 * Currently supports:
 *
 * - Non-POSIX (not part of POSIX standard for various reasons)
 */
struct bsem {
  struct mutex mtx;
  struct condv cv;
  bool_t       val;

  /**
   * \brief Incremented by \ref bsem_flush(); waiters that see it change are
   * released without taking the semaphore.
   */
  uint32_t flush_gen;

  /**
   * \brief Configuration flags.
   *
   * Valid flags are:
   *
   * - \ref RCSW_ZALLOC
   * - \ref RCSW_NOALLOC_HANDLE
   */
  uint32_t flags;
};

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Initialize a binary semaphore.
 *
 * \param sem_in Caller storage for the handle, used only if \ref
 *               RCSW_NOALLOC_HANDLE is passed; ignored (may be NULL)
 *               otherwise. See \rcswdoc{concepts/memory-model}.
 *
 * \param flags Configuration flags. See \ref bsem.flags for valid flags.
 *
 * \return The initialized binary semaphore, or NULL if an ERROR occurred.
 */
RCSW_API struct bsem* bsem_init(struct bsem* sem_in, uint32_t flags);

/**
 * \brief Destroy a binary semaphore.
 *
 * Any further use of the semaphore after calling this function is undefined.
 *
 * \param sem The semaphore to destroy.
 */
RCSW_API void bsem_destroy(struct bsem* sem);

/**
 * \brief Unlock a binary semaphore.
 *
 * \param sem The semaphore handle.
 *
 * \return \ref status_t.
 */
RCSW_API status_t bsem_post(struct bsem* sem);

/**
 * \brief - Notify all waiting threads and make it available again.
 *
 * \param sem The semaphore handle.
 *
 * \return \ref status_t.
 */
RCSW_API status_t bsem_flush(struct bsem* sem);

/**
 * \brief Wait on binary semaphore with a timeout.
 *
 * \param sem The semaphore handle.
 *
 * \param to A relative timeout. See
 *           \rcswdoc{concepts/concurrency/timeouts}.
 *
 * \return \ref status_t.
 */
RCSW_API status_t bsem_timedwait(struct bsem* sem, const struct timespec* to);

/**
 * \brief Block on binary semaphore until it becomes available.
 *
 * \param sem The semaphore handle.
 *
 * \return \ref status_t.
 */
RCSW_API status_t bsem_wait(struct bsem* sem);

END_C_DECLS
