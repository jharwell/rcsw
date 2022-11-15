/**
 * \file mt_bsem.h
 * \ingroup multithread
 * \brief Binary semaphore implemented using mutex and cv.
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

#ifndef INCLUDE_RCSW_MULTITHREAD_MT_BSEM_H_
#define INCLUDE_RCSW_MULTITHREAD_MT_BSEM_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <time.h>
#include "rcsw/common/common.h"
#include "rcsw/multithread/mt.h"
#include "rcsw/multithread/mt_cond.h"
#include "rcsw/multithread/mt_mutex.h"

/*******************************************************************************
 * Type Definitions
 ******************************************************************************/
/**
 * \brief Binary semaphore implementation using a mutex and condition
 * variable. This is not provided on linux.
 */
typedef struct {
    mt_mutex_t mutex;
    mt_cond_t cv;
    size_t val;
    uint32_t flags;
} mt_bsem_t;

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Initialize a binary semaphore.
 *
 * \param sem_in The semaphore to initialize. Can be NULL if \ref
 * MT_APP_DOMAIN_MEM is not passed.
 * \param flags Initialization flags
 *
 * \return The initialized binary semaphore, or NULL if an ERROR occurred.
 */
mt_bsem_t* mt_bsem_init(mt_bsem_t * sem_in, uint32_t flags);

/**
 * \brief Destroy a binary semaphore.
 *
 * Any further use of the semaphore after calling this function is undefined.
 *
 * \param sem_p The semaphore to destroy.
 */
void mt_bsem_destroy(mt_bsem_t * sem_p);

/**
 * \brief Unlock a binary semaphore.
 *
 * \param sem_p The semaphore handle.
 *
 * \return \ref status_t.
 */
status_t mt_bsem_post(mt_bsem_t * sem_p);

/**
 * \brief - Notify whoever is waiting on the semaphore and make it available
 * again.
 *
 * \param sem_p The semaphore handle.
 *
 * \return \ref status_t.
 */
status_t mt_bsem_flush(mt_bsem_t * sem_p);

/**
 * \brief Wait on binary semaphore with a timeout.
 *
 * \param sem_p The semaphore handle.
 * \param to  A RELATIVE timeout, NOT an ABSOLUTE timeout, as the POSIX standard
 * specifies. This function converts the relative timeout to absolute timeout
 * required.
 *
 * \return \ref status_t.
 */
status_t mt_bsem_timedwait(mt_bsem_t * sem_p,
                           const struct timespec * to);

/**
 * \brief Block on binary semaphore until it becomes available.
 *
 * \param sem_p The semaphore handle.
 *
 * \return \ref status_t.
 */
status_t mt_bsem_wait(mt_bsem_t * sem_p);

END_C_DECLS

#endif /* INCLUDE_RCSW_MULTITHREAD_MT_BSEM_H_ */
