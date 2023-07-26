/**
 * \file cvm.h
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
#include <pthread.h>

#include "rcsw/multithread/mutex.h"
#include "rcsw/multithread/condv.h"
#include "rcsw/rcsw.h"

/*******************************************************************************
 * Type Definitions
 ******************************************************************************/
/**
 * \brief Convenience wrapper \ref condv and \ref mutex, because they often used
 * together.
 *
 * Currently supports:
 *
 * - POSIX
 */
struct cvm {
  struct mutex mtx;
  struct condv cv;

  /**
   * Valid flags are:
   *
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
 * \brief Initialize the signal condition (variable and mutex).
 *
 * \param cvm_in CVM to initialize. Can be NULL if \ref RCSW_NOALLOC_HANDLE
 *               passed.
 *
 * \param flags Configuration flags.
 *
 * \return The initialized CVM, or NULL if an ERROR occurred.
 */
struct cvm* cvm_init(struct cvm * cvm_in, uint32_t flags);

/**
 * \brief Destroy the signal condition.
 *
 * \param cvm The CVM handle.
 */
void mt_cvm_destroy(struct cvm * cvm);

/**
 * \brief Signal on a condition variable while holding a mutex.
 *
 * \param cvm The CVM handle.
 *
 * \return \ref status_t.
 */
status_t cvm_signal(struct cvm * cvm);

/**
 * \brief Broadcast to everyone waiting on a condition variable.
 *
 * This function unblocks all threads currently blocked on the condition
 * variable. Each thread, upon its return from \ref cvm_wait() or \ref
 * cvm_timedwait() will own the mutex it entered its waiting function with.
 *
 * \param cvm The CVM handle.
 *
 * \return \ref status_t.
 */
status_t cvm_broadcast(struct cvm * cvm);

/**
 * \brief  Wait on a condition variable while holding a mutex.
 *
 * \param cvm The CVM handle.
 *
 * \return \ref status_t.
 */
status_t cvm_wait(struct cvm * cvm);

/**
 * \brief Timed wait on a condition variable while holding a mutex.
 *
 * \param cvm The CVM handle.
 *
 * \param to A RELATIVE timeout, NOT an ABSOLUTE timeout, as the POSIX standard
 *            specifies. This function converts the relative timeout to absolute
 *            timeout required.
 *
 * \return \ref status_t
 */
status_t cvm_timedwait(struct cvm * cvm, const struct timespec * to);

END_C_DECLS
