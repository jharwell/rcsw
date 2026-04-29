/**
 * \file utils_test_utils.hpp
 *
 * Shared helpers for utils unit tests.
 *
 * Provides capture_stdout() — a thin pipe/dup2 wrapper that captures
 * everything written to fd 1 (including printf/DPRINTF output) within the
 * scope of a lambda, and RCSW_ARRAY_ELTS for C-array sizing.
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

#include "rcsw/core/core.h"

/*******************************************************************************
 * Free Functions
 ******************************************************************************/

/**
 * \brief Execute \p fn and return everything it wrote to stdout (fd 1).
 *
 * Flushes stdout before installing the pipe so that any buffered Catch2
 * header output is drained before the capture window opens.
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
