/**
 * \file rdwrlock.h
 * \ingroup multithread
 * \brief Implementation of fair reader-writer lock.
 *
 * Guarantees that no reader or writer will starve.
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/multithread/csem.h"
#include "rcsw/rcsw.h"

/*******************************************************************************
 * Type Definitions
 ******************************************************************************/
/**
 * \brief Fair reader-writer lock that guarantees that neither readers nor
 * writers will starve.
 */
struct rdwrlock {
  /** Used to form a queue of readers/writers */
  struct csem order;

  /** Exclusive access semaphore */
  struct csem access;

  /** Semaphore for updating n_readers */
  struct csem read;

  /** Number of readers currently accessing resource */
  size_t n_readers;

  /**
   * Configuration flags. Valid flags are:
   *
   * - \ref RCSW_NOALLOC_HANDLE
   *
   * All other flags are ignored.
   */
  uint32_t flags;
};

/**
 * \brief The scope of privileges requested for a critical section of code:
 * reading or writing.
 *
 * For use with \ref rdwrlock, in which there can be multiple concurrent readers
 * OR a single writer in a critical section at a given time.
 */
enum rdwrlock_scope {
  ekSCOPE_RD,
  ekSCOPE_WR,
};

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Initialize a reader/writer fair lock
 *
 * \param rdwr_in Lock to initialize. Can be NULL if \ref RCSW_NOALLOC_HANDLE
 *                passed.
 *
 * \param flags Configuration flags
 *
 * \return The initialized RDWR lock, or NULL if an ERROR occurred.
 */
struct rdwrlock* rdwrl_init(struct rdwrlock *const rdwr_in,
                            uint32_t flags) RCSW_CHECK_RET;

/**
 * \brief Destroy a reader/writer fair lock
 *
 * Any further use of the lock after calling this function is undefined.
 *
 * \param rdwr The lock handle.
 */
void rdwrl_destroy(struct rdwrlock *const rdwr);


/**
 * \brief Request to enter a critical section.
 *
 * This function is part of a fair (i.e. no rd/wr preference) sequence that
 * guarantees that no reader or write will starve.
 *
 * \param rdwr The lock handle.
 *
 * \param scope The scope of the privileges requested.
 */
void rdwrl_req(struct rdwrlock *const rdwr, enum rdwrlock_scope scope);

/**
 * \brief Exit a critical section.
 *
 * This function is part of a fair (i.e. no rd/wr preference) sequence
 * that guarantees that no reader or write will starve.
 *
 * \param rdwr The lock handle.
 *
 * \param scope The scope of the privileges requested.
 */
void rdwrl_exit(struct rdwrlock *const rdwr, enum rdwrlock_scope scope);

/**
 * \brief Request to enter a critical section with a timeout.
 *
 * This function is part of a fair (i.e. no rd/wr preference) sequence
 * that guarantees that no reader or write will starve.
 *
 * \param rdwr The lock handle.
 *
 * \param scope The scope of the privileges requested.
 *
 * \param to A RELATIVE timeout, NOT an ABSOLUTE timeout, as the POSIX standard
 *           specifies. This function converts the relative timeout to absolute
 *           timeout required.
 *
 * \return \ref status_t.
 */
status_t rdwrl_timedreq(struct rdwrlock *const rdwr,
                        enum rdwrlock_scope scope,
                        const struct timespec *const to);

END_C_DECLS
