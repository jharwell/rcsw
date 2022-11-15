/**
 * \file mt_csem.h
 * \ingroup multithread
 * \brief Wrapper for OS sem_t.
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * This file is part of RCSW.
 *
 * RCSW is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * RCSW is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * RCSW.  If not, see <http://www.gnu.org/licenses/
 */

#ifndef INCLUDE_RCSW_MULTITHREAD_MT_CSEM_H_
#define INCLUDE_RCSW_MULTITHREAD_MT_CSEM_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <semaphore.h>
#include "rcsw/multithread/mt.h"
#include "rcsw/multithread/mt_mutex.h"
#include "rcsw/common/common.h"

/*******************************************************************************
 * Type Definitions
 ******************************************************************************/
/**
 * \brief Wrapper around POSIX semaphores. In the future, this may
 * incorporate semaphores from other operating systems.
 */
typedef struct {
    sem_t sem;
    uint32_t flags;
} mt_csem_t;

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Initialize a counting semaphore
 *
 * \param sem_in semaphore to initialize. Can be NULL if \ref MT_APP_DOMAIN_MEM
 * passed.
 * \param shared FALSE - shared between threads of a process; TRUE - shared
 * between processes.
 * \param value The initial semaphore value.
 * \param flags Configuration flags.
 *
 * \return The initialization counting semaphore, or NULL if an ERROR occurred.
 */
mt_csem_t* mt_csem_init(mt_csem_t *sem_in, bool_t shared, size_t value,
                        uint32_t flags);

/**
 * \brief Destroy a counting semaphore.
 *
 * \param sem The semaphore to destroy.
 */
void mt_csem_destroy(mt_csem_t * sem);

/**
 * \brief Increment (unlock) a counting semaphore.
 *
 * \param sem The semaphore handle.
 *
 * \return \ref status_t.
 */
status_t mt_csem_post(mt_csem_t * sem);

/**
 * mt_csem_timedwait() - Wait on (lock) a counting semaphore with a timeout.
 *
 * \param sem The semaphore handle.
 * \param to A RELATIVE timeout, NOT an ABSOLUTE timeout, as the POSIX standard
 * specifies. This function converts the relative timeout to absolute timeout
 * required.
 *
 * \return \ref status_t.
 */
status_t mt_csem_timedwait(mt_csem_t * sem,
                           const struct timespec * to);

/**
 * \brief Wait on (lock) a counting semaphore.
 *
 * \param sem The semaphore handle.
 *
 * \return \ref status_t.
 */
status_t mt_csem_wait(mt_csem_t *sem);

/**
 * \brief Lock a counting semaphore only if it is currently
 * available. Otherwise, do nothing.
 *
 * \param sem The semaphore handle.
 *
 * \return \ref status_t.
 */
status_t mt_csem_trywait(mt_csem_t *sem);

END_C_DECLS

#endif /* INCLUDE_RCSW_MULTITHREAD_MT_CSEM_H_ */
