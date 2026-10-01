/**
 * \file
 *
 * \copyright 2017 John Harwell
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#ifndef _GNU_SOURCE
/* This is required to get CPU_ZERO() and friends */
/* NOLINTNEXTLINE(bugprone-reserved-identifier,cert-dcl37-c) */
#define _GNU_SOURCE
#endif

#include "rcsw/multiprocess/procm.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>

#include "rcsw/core/fpc.h"
#include "rcsw/er/client.h"

/*******************************************************************************
 * Constants
 ******************************************************************************/
/* The sysfs file giving the socket (physical package) a CPU belongs to. */
#define PROCM_PACKAGE_ID_FMT \
  "/sys/devices/system/cpu/cpu%ld/topology/physical_package_id"

/*******************************************************************************
 * Private API
 ******************************************************************************/
BEGIN_C_DECLS
/**
 * \brief Get the socket (physical package) that CPU \p cpu is on.
 *
 * \return The socket, or -1 if the CPU doesn't exist or is offline (its
 * topology isn't in sysfs then).
 */
static long procm_cpu_socket(long cpu) {
  /* The format's "%ld" leaves room for any long. */
  char path[sizeof(PROCM_PACKAGE_ID_FMT) + 20];
  int  len = snprintf(path, sizeof(path), PROCM_PACKAGE_ID_FMT, cpu);
  if (len < 0 || (size_t)len >= sizeof(path)) {
    return -1;
  }

  FILE* f = fopen(path, "re");
  if (NULL == f) {
    return -1;
  }
  char  buffer[32];
  char* line = fgets(buffer, sizeof(buffer), f);
  (void)fclose(f);
  if (NULL == line) {
    return -1;
  }

  char* end   = NULL;
  errno       = 0;
  long socket = strtol(line, &end, 10);
  if (0 != errno || end == line || socket < 0) {
    return -1;
  }
  return socket;
}

/**
 * \brief End the fork()ed child after a failure before exec().
 *
 * Uses _exit() rather than exit(): exit() would also run the parent's atexit()
 * handlers and flush the parent's unflushed stdio buffers a second time.
 */
static void procm_child_fail(const char* msg, size_t len) {
  /* Only async-signal-safe calls are allowed between fork() and exec(), so
   * write() rather than stdio. There's nothing to do if it fails; glibc warns
   * about an unused result even when cast to void, hence the variable. */
  ssize_t rc = write(STDERR_FILENO, msg, len);
  (void)rc;
  _exit(EXIT_FAILURE);
}

#define PROCM_CHILD_FAIL(msg) procm_child_fail((msg), sizeof(msg) - 1)

/******************************************************************************
 * Public API
 ******************************************************************************/
status_t procm_socket_lock(int socket) {
  RCSW_FPC_NV(ERROR, socket >= 0);

  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);

  /*
   * Use the configured rather than the online CPU count: CPU numbers can have
   * gaps where CPUs are offline, and a CPU's number can be larger than the
   * number of CPUs online.
   */
  long n_cpus = sysconf(_SC_NPROCESSORS_CONF);
  RCSW_CHECK(n_cpus > 0);
  if (n_cpus > CPU_SETSIZE) {
    n_cpus = CPU_SETSIZE;
  }

  /*
   * Ask each CPU which socket it's on, rather than assuming each socket's CPUs
   * are numbered consecutively; many machines interleave them.
   */
  size_t n_set = 0;
  for (long cpu = 0; cpu < n_cpus; ++cpu) {
    if (procm_cpu_socket(cpu) == socket) {
      CPU_SET((size_t)cpu, &cpuset);
      ++n_set;
    }
  } /* for(cpu..) */

  /* No CPUs: there's no such socket, or none of its CPUs are online. */
  if (0 == n_set) {
    errno = EINVAL;
    return ERROR;
  }

  RCSW_CHECK(0 == sched_setaffinity(0, sizeof(cpuset), &cpuset));
  return OK;

error:
  return ERROR;
}

pid_t procm_fork_exec(char** const cmd,
                      const char*  new_wd,
                      bool_t       stdout_sup,
                      int*         pipefd) {
  RCSW_FPC_NV(-1, NULL != cmd, NULL != cmd[0]);

  pid_t pid = fork();
  if (0 != pid) {
    /* The parent, or fork() failed (-1, with errno set). */
    return pid;
  }

  /* change to the working directory before exec()ing if requested */
  if (NULL != new_wd && 0 != chdir(new_wd)) {
    PROCM_CHILD_FAIL("procm_fork_exec: chdir() failed\n");
  }

  /* suppress stdout */
  if (stdout_sup) {
    int fd = open("/dev/null", O_WRONLY | O_CLOEXEC);
    if (fd < 0 || dup2(fd, STDOUT_FILENO) < 0) {
      PROCM_CHILD_FAIL("procm_fork_exec: redirecting stdout failed\n");
    }
    if (STDOUT_FILENO != fd) {
      (void)close(fd);
    }
  }

  /* the child will read data on stdin from the parent */
  if (NULL != pipefd) {
    if (dup2(pipefd[0], STDIN_FILENO) < 0) {
      PROCM_CHILD_FAIL("procm_fork_exec: redirecting stdin failed\n");
    }
    /*
     * stdin is now the read end, so close both originals. The write end
     * especially: while the child holds it open, reading stdin never reaches
     * EOF, even after the parent closes its copy.
     */
    if (STDIN_FILENO != pipefd[0]) {
      (void)close(pipefd[0]);
    }
    (void)close(pipefd[1]);
  }

  execv(cmd[0], cmd);

  /* execv() only returns on failure. */
  PROCM_CHILD_FAIL("procm_fork_exec: execv() failed\n");
  return -1; /* not reached */
}

END_C_DECLS
