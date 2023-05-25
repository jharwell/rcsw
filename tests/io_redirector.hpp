/**
 * \file io_redirector.hpp
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <unistd.h>
#include <fcntl.h>

#include <string>

/*******************************************************************************
 * Namespaces/Decls
 ******************************************************************************/

/*******************************************************************************
 * Class Definitions
 ******************************************************************************/
class io_redirector {
 public:
  io_redirector(void) = default;
  virtual ~io_redirector(void) = default;

  virtual void start(void) = 0;
  virtual void stop(void) = 0;

  virtual std::string get_output(void) = 0;
  virtual void clear_output(void) = 0;
};


class cstdout_redirector : public io_redirector {
 public:
  enum PIPES { ekREAD, ekWRITE };

  cstdout_redirector(void) {
    m_pipe[ekREAD] = 0;
    m_pipe[ekWRITE] = 0;

    if (pipe(m_pipe) != -1) {
      m_stdout_old = dup(fileno(stdout));
      m_stderr_old = dup(fileno(stderr));
    }

    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
  }

  ~cstdout_redirector(void) {
    if (m_redirecting) {
      stop();
    }

    if (m_stdout_old > 0) {
      close(m_stdout_old);
    }
    if (m_stderr_old > 0) {
      close(m_stderr_old);
    }

    if (m_pipe[ekREAD] > 0) {
      close(m_pipe[ekREAD]);
    }
    if (m_pipe[ekWRITE] > 0) {
      close(m_pipe[ekWRITE]);
    }
  }

  void start() override {
    if (m_redirecting) {
      return;
    }
    dup2(m_pipe[ekWRITE], fileno(stdout));
    dup2(m_pipe[ekWRITE], fileno(stderr));
    m_redirecting = true;
  }


  void stop() override {
    if (!m_redirecting) {
      return;
    }

    dup2(m_stdout_old, fileno(stdout));
    dup2(m_stderr_old, fileno(stderr));
    m_redirecting = false;
  }

  std::string get_output(void) override {
    fcntl(m_pipe[ekREAD], F_SETFL, O_NONBLOCK);
    memset(m_output_buf, 0, sizeof(m_output_buf));
    ssize_t n_bytes = read(m_pipe[ekREAD],
                           m_output_buf,
                           sizeof(m_output_buf) - 1);

    while (n_bytes > 0) {
      m_output_buf[n_bytes] = 0;
      m_redirected_output += m_output_buf;
      n_bytes = read(m_pipe[ekREAD], m_output_buf, sizeof(m_output_buf));
    }

    return m_redirected_output;
  }


  void clear_output(void) override {
    m_redirected_output.clear();
  }

 private:
  int         m_pipe[2];
  int         m_stdout_old{0};
  int         m_stderr_old{0};
  bool        m_redirecting{false};
  char        m_output_buf[4096];

  std::string m_redirected_output{};
};
