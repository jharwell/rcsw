/**
 * \file bsem.h
 * \ingroup multithread
 * \brief Binary semaphore implemented using mutex and cv.
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <time.h>
#include "rcsw/rcsw.h"
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
  bool_t val;

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
 * \param sem_in The semaphore to initialize. Can be NULL if \ref
 *               RCSW_NOALLOC_HANDLE is not passed.
 *
 * \param flags Initialization flags
 *
 * \return The initialized binary semaphore, or NULL if an ERROR occurred.
 */
struct bsem* bsem_init(struct bsem * sem_in, uint32_t flags);

/**
 * \brief Destroy a binary semaphore.
 *
 * Any further use of the semaphore after calling this function is undefined.
 *
 * \param sem The semaphore to destroy.
 */
void bsem_destroy(struct bsem * sem);

/**
 * \brief Unlock a binary semaphore.
 *
 * \param sem The semaphore handle.
 *
 * \return \ref status_t.
 */
status_t bsem_post(struct bsem * sem);

/**
 * \brief - Notify all waiting threads and make it available again.
 *
 * \param sem The semaphore handle.
 *
 * \return \ref status_t.
 */
status_t bsem_flush(struct bsem * sem);

/**
 * \brief Wait on binary semaphore with a timeout.
 *
 * \param sem The semaphore handle.
 *
 * \param to A RELATIVE timeout, NOT an ABSOLUTE timeout, as the POSIX standard
 *           specifies. This function converts the relative timeout to absolute
 *           timeout required.
 *
 * \return \ref status_t.
 */
status_t  bsem_timedwait(struct bsem * sem, const struct timespec * to);

/**
 * \brief Block on binary semaphore until it becomes available.
 *
 * \param sem The semaphore handle.
 *
 * \return \ref status_t.
 */
status_t bsem_wait(struct bsem * sem);

END_C_DECLS
