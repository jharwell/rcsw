/**
 * \file er_test_utils.hpp
 *
 * Shared helpers for ER unit tests.
 *
 * Provides capture_stdout() – a thin wrapper around pipe/dup2 that captures
 * everything written to fd 1 (including printf output from the SIMPLE plugin)
 * within the scope of a lambda.
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <functional>
#include <string>
#include <unistd.h>

/*******************************************************************************
 * Public API
 ******************************************************************************/
/**
 * \brief Execute \p fn and return everything it wrote to stdout (fd 1).
 *
 * Flushes stdout before redirecting so that any buffered output already
 * produced by Catch2 (e.g. the "Randomness seeded to:" line) is drained to
 * the real file descriptor before the pipe is installed.  Only output
 * produced during the execution of \p fn is returned.
 *
 * Works for both printf-style (fd 1) and std::cout output.
 */
static inline std::string capture_stdout(std::function<void()> fn) {
  fflush(stdout);

  int pipefd[2];
  pipe(pipefd);
  int saved = dup(STDOUT_FILENO);
  dup2(pipefd[1], STDOUT_FILENO);
  close(pipefd[1]);

  fn();
  fflush(stdout);

  dup2(saved, STDOUT_FILENO);
  close(saved);

  std::string result;
  char        buf[4096];
  ssize_t     n;
  while ((n = read(pipefd[0], buf, sizeof(buf))) > 0) {
    result.append(buf, n);
  }
  close(pipefd[0]);
  return result;
}
