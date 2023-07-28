/**
 * \file csem.h
 * \ingroup multithread
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <semaphore.h>

#include "rcsw/multithread/mutex.h"
#include "rcsw/rcsw.h"

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
 * \param sem_in Semaphore to initialize. Can be NULL if \ref
 *                RCSW_NOALLOC_HANDLE passed.
 *
 * \param value The initial semaphore value.
 *
 * \param flags Configuration flags.
 *
 * \return The initialization counting semaphore, or NULL if an ERROR occurred.
 */
struct csem* csem_init(struct csem *sem_in,
                          size_t value,
                          uint32_t flags);

/**
 * \brief Destroy a counting semaphore.
 *
 * \param sem The semaphore to destroy.
 */
void csem_destroy(struct csem * sem);

/**
 * \brief Increment (unlock) a counting semaphore.
 *
 * \param sem The semaphore handle.
 *
 * \return \ref status_t.
 */
status_t csem_post(struct csem * sem);

/**
 * struct csemimedwait() - Wait on (lock) a counting semaphore with a timeout.
 *
 * \param sem The semaphore handle.
 *
 * \param to A RELATIVE timeout, NOT an ABSOLUTE timeout, as the POSIX standard
 *           specifies. This function converts the relative timeout to absolute
 *           timeout required.
 *
 * \return \ref status_t.
 */
status_t csem_timedwait(struct csem * sem, const struct timespec * to);

/**
 * \brief Wait on (lock) a counting semaphore.
 *
 * \param sem The semaphore handle.
 *
 * \return \ref status_t.
 */
status_t csem_wait(struct csem *sem);

/**
 * \brief Lock the semaphore only if it is currently available.
 *
 * Otherwise, do nothing.
 *
 * \param sem The semaphore handle.
 *
 * \return \ref status_t.
 */
status_t csem_trywait(struct csem *sem);

END_C_DECLS
