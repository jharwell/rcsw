/**
 * \file procm.h
 * \ingroup multiprocess
 * \brief Useful routines related to fork()/exec() process management.
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

#ifndef INCLUDE_RCSW_MULTIPROCESS_PROCM_H_
#define INCLUDE_RCSW_MULTIPROCESS_PROCM_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/common/common.h"

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Lock a process to a particular CPU socket.
 *
 * \param  socket The socket to lock to, 0 indexed.
 *
 * \return OK if the lock with successful, ERROR otherwise.
 */
status_t procm_socket_lock(int socket);

/**
 * \brief Wrapper for fork()/exec() functonality.
 *
 * \param cmd The cmd to run exec() on
 * \param new_wd The new working directory of the fork()ed process, or NULL if
 * no change is desired.
 * \param stdout_sup If TRUE, then the stdout of the child process will be sent
 * to /dev/null.
 * \param pipefd If not NULL, the child will read data from the parent's stdin.
 * \return The pid of the child in the parent, nothing in the child.
 */
pid_t procm_fork_exec(char** const cmd, const char* new_wd,
                      bool_t stdout_sup, int* pipefd);

END_C_DECLS

#endif /* INCLUDE_RCSW_MULTIPROCESS_PROCM_H_ */
