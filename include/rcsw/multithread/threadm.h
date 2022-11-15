/**
 * \file threadm.h
 * \ingroup multithread
 * \brief Various thread management tools
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

#ifndef INCLUDE_RCSW_MULTITHREAD_THREADM_H_
#define INCLUDE_RCSW_MULTITHREAD_THREADM_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/common/common.h"

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Lock a thread to a core.
 *
 * \param thread The thread handle.
 * \param core The core to lock to, 0-indexed.
 *
 * \return \ref status_t.
 */
status_t threadm_core_lock(pthread_t thread, size_t core);

END_C_DECLS

#endif /* INCLUDE_RCSW_MULTITHREAD_THREADM_H_ */
