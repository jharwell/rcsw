/**
 * \file
 *
 * \copyright 2017 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * \ingroup multithread
 *
 * \brief Counting semaphore.
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <semaphore.h>
#include <stdint.h>
#include <time.h>

#include "rcsw/core/compilers.h"
#include "rcsw/core/core.h"

/*******************************************************************************
 * Type Definitions
 ******************************************************************************/
/**
 * \brief Wrapper around counting semaphores from various implementations.
 *
 * Currently supports:
 *
 * - POSIX
 */
struct csem {
  sem_t impl;

  /**
   * Valid flags are:
   *
   * - \ref RCSW_ZALLOC
   * - \ref RCSW_NOALLOC_HANDLE
   *
   * All other flags are ignored.
   */
  uint32_t flags;
};

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Initialize a counting semaphore
 *
 * \param sem_in Caller storage for the handle, used only if \ref
 *               RCSW_NOALLOC_HANDLE is passed; ignored (may be NULL)
 *               otherwise. See \rcswdoc{concepts/memory-model}.
 *
 * \param value The initial semaphore value.
 *
 * \param flags Configuration flags. See \ref csem.flags for valid flags.
 *
 * \return The initialization counting semaphore, or NULL if an ERROR occurred.
 */
RCSW_API struct csem* csem_init(struct csem* sem_in,
                                size_t       value,
                                uint32_t     flags);

/**
 * \brief Destroy a counting semaphore.
 *
 * \param sem The semaphore to destroy.
 */
RCSW_API void csem_destroy(struct csem* sem);

/**
 * \brief Increment (unlock) a counting semaphore.
 *
 * \param sem The semaphore handle.
 *
 * \return \ref status_t.
 */
RCSW_API status_t csem_post(struct csem* sem);

/**
 * Wait on a counting semaphore with a timeout.
 *
 * \param sem The semaphore handle.
 *
 * \param to A relative timeout. See
 *           \rcswdoc{concepts/concurrency/timeouts}.
 *
 * \return \ref status_t.
 */
RCSW_API status_t csem_timedwait(struct csem* sem, const struct timespec* to);

/**
 * Wait on a counting semaphore with a timeout.
 *
 * \param sem The semaphore handle.
 *
 * \param to An absolute \c CLOCK_REALTIME deadline. See
 *           \rcswdoc{concepts/concurrency/timeouts}.
 *
 * \return \ref status_t.
 */
RCSW_API status_t csem_timedwait_abs(struct csem* sem, const struct timespec* to);

/**
 * \brief Wait on (lock) a counting semaphore.
 *
 * \param sem The semaphore handle.
 *
 * \return \ref status_t.
 */
RCSW_API status_t csem_wait(struct csem* sem);

/**
 * \brief Lock the semaphore only if it is currently available.
 *
 * Otherwise, do nothing.
 *
 * \param sem The semaphore handle.
 *
 * \return \ref status_t.
 */
RCSW_API status_t csem_trywait(struct csem* sem);

END_C_DECLS
