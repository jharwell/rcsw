/**
 * \file procm.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#define _GNU_SOURCE
#include "rcsw/multiprocess/procm.h"

#include <assert.h>
#include <fcntl.h>
#include <sched.h>
#include <unistd.h>

#include "rcsw/common/dbg.h"

/*******************************************************************************
 * Forward Declarations
 ******************************************************************************/
BEGIN_C_DECLS

/*******************************************************************************
 * Functions
 ******************************************************************************/
status_t procm_socket_lock(int socket) {
  cpu_set_t cpuset;
  char buffer[50];
  char* line;
  int64_t n_sockets, cores_per_socket, n_cpus;

  CPU_ZERO(&cpuset);
  FILE* f = popen("lscpu | grep Socket|awk '{print $2}'", "r");
  RCSW_CHECK_PTR(f);

  line = fgets(buffer, sizeof(buffer), f);
  RCSW_CHECK_PTR(line);
  pclose(f);

  n_cpus = sysconf(_SC_NPROCESSORS_ONLN);
  n_sockets = atoi(line);
  cores_per_socket = n_cpus / n_sockets;

  for (int64_t i = socket * cores_per_socket; i < (socket + 1) * cores_per_socket;
       ++i) {
    CPU_SET(i, &cpuset);
  } /* for(i..) */

  RCSW_CHECK(0 == sched_setaffinity(0, sizeof(cpu_set_t), &cpuset));
  return OK;

error:
  return ERROR;
} /* procm_socket_lock() */

pid_t procm_fork_exec(char** const cmd,
                      const char* new_wd,
                      bool_t stdout_sup,
                      int* pipefd) {
  pid_t pid = fork();
  if (0 == pid) {
    /* change to the working directory before exec()ing if requested */
    if (NULL != new_wd) {
      if (0 != chdir(new_wd)) {
        return ERROR;
      }
    }

    /* suppress stdout */
    if (stdout_sup) {
      int fd = open("/dev/null", O_WRONLY);
      dup2(fd, 1);
    }

    /* the child will read data on stdin from the parent */
    if (NULL != pipefd) {
      dup2(pipefd[0], STDIN_FILENO);
    }
    execv(cmd[0], cmd);
    perror(NULL);
    assert(0); /* execv() should never return */
    exit(EXIT_FAILURE);
  } else {
    return pid;
  }
} /* procm_fork_exec() */

END_C_DECLS
