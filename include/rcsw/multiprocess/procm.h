/**
 * \file procm.h
 * \ingroup multiprocess
 * \brief Useful routines related to fork()/exec() process management.
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/rcsw.h"

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

