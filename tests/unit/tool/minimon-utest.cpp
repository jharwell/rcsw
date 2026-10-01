/**
 * \file
 *
 * \copyright 2026 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * Tests for minimon.
 *
 * minimon_start() never returns, so each test feeds command lines through a
 * pipe on stdin and registers a custom command whose hook records its argument
 * and longjmp()s back to the test. Every input must end with a "finish"
 * command.
 *
 * Requires RCSW_CONFIG_STDIO_GETCHAR to read from stdin (the default).
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#define CATCH_CONFIG_PREFIX_ALL
#include <csetjmp>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <functional>

#include <fcntl.h>
#include <unistd.h>

#include <catch2/catch_test_macros.hpp>

#include "rcsw/core/core.h"
#include "rcsw/console/minimon.h"

/*******************************************************************************
 * Helpers
 ******************************************************************************/
namespace {
std::jmp_buf g_done;
uint32_t     g_arg;

/* The hook receives `union minimon_cmd_arg` values via varargs. */
void finish_hook(const char*, ...) {
  va_list args;
  va_start(args, nullptr);
  union minimon_cmd_arg a = va_arg(args, union minimon_cmd_arg);
  va_end(args);
  g_arg = a.num;
  std::longjmp(g_done, 1);
}

struct minimon_cmd make_finish_cmd() {
  struct minimon_cmd cmd;
  memset(&cmd, 0, sizeof(cmd));
  cmd.name                 = "finish";
  cmd.alias                = "fin";
  cmd.help                 = "End the test";
  cmd.hook                 = finish_hook;
  cmd.config[0].name       = "n";
  cmd.config[0].type       = MINIMON_PARAM_UINT32;
  cmd.config[0].short_help = "value";
  cmd.config[0].long_help  = "value";
  cmd.config[0].required   = true;
  return cmd;
}

/*
 * Run the monitor with \p input on stdin (stdout silenced) until the
 * "finish" command fires. Returns the argument "finish" received.
 *
 * \p config defaults to the built-ins plus a single "finish" command;
 * \p after_init runs between minimon_init() and minimon_start().
 */
uint32_t run_monitor(const char*                  input,
                     const struct minimon_config* config     = nullptr,
                     const std::function<void()>& after_init = {}) {
  int fds[2];
  CATCH_REQUIRE(0 == pipe(fds));
  CATCH_REQUIRE(static_cast<ssize_t>(strlen(input)) ==
                write(fds[1], input, strlen(input)));
  close(fds[1]);

  fflush(stdout);
  int saved_in  = dup(STDIN_FILENO);
  int saved_out = dup(STDOUT_FILENO);
  int devnull   = open("/dev/null", O_WRONLY);
  dup2(fds[0], STDIN_FILENO);
  dup2(devnull, STDOUT_FILENO);
  close(fds[0]);
  close(devnull);

  static struct minimon_cmd cmds[1];
  cmds[0] = make_finish_cmd();

  struct minimon_config c;
  memset(&c, 0, sizeof(c));
  c.cmds            = cmds;
  c.n_cmds          = 1;
  c.include_builtin = true;
  c.help_on_start   = false;

  g_arg = 0;
  if (0 == setjmp(g_done)) {
    minimon_init(nullptr != config ? config : &c);
    if (after_init) {
      after_init();
    }
    minimon_start(); /* returns only via finish_hook() */
  }

  fflush(stdout);
  clearerr(stdin);
  dup2(saved_in, STDIN_FILENO);
  dup2(saved_out, STDOUT_FILENO);
  close(saved_in);
  close(saved_out);
  return g_arg;
}
} /* namespace */

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("minimon dispatches a custom command with its argument",
                "[tool][minimon][noalloc]") {
  CATCH_REQUIRE(5 == run_monitor("finish 5\n"));
}

CATCH_TEST_CASE(
  "minimon survives a command line with more arguments than "
  "MINIMON_CMD_MAX_ARGS",
  "[tool][minimon][noalloc]") {
  /* the long line is rejected; the next command still dispatches */
  CATCH_REQUIRE(5 == run_monitor("help 1 2 3 4 5 6 7 8 9 10\nfinish 5\n"));
}

CATCH_TEST_CASE("minimon matches command names exactly, not by prefix",
                "[tool][minimon][noalloc]") {
  /* "finis" and "fi" are neither the name nor the alias */
  CATCH_REQUIRE(7 == run_monitor("finis 5\nfi 6\nfinish 7\n"));
  CATCH_REQUIRE(8 == run_monitor("fin 8\n")); /* the alias */
}

CATCH_TEST_CASE("minimon ignores backspace on an empty line",
                "[tool][minimon][noalloc]") {
  CATCH_REQUIRE(3 == run_monitor("\bfinish 3\n"));
  CATCH_REQUIRE(4 == run_monitor("finish 9\b4\n"));
}

CATCH_TEST_CASE("minimon uses the caller's command table in place",
                "[tool][minimon][noalloc]") {
  static struct minimon_cmd cmds[1];
  cmds[0] = make_finish_cmd();

  struct minimon_config c;
  memset(&c, 0, sizeof(c));
  c.cmds            = cmds;
  c.n_cmds          = 1;
  c.include_builtin = true;

  /* Renamed after minimon_init(): only the new name matches, so no copy */
  CATCH_REQUIRE(2 == run_monitor("finish 1\ndone 2\n", &c, [] {
                  cmds[0].name = "done";
                }));
}

CATCH_TEST_CASE("minimon has no limit on the number of user commands",
                "[tool][minimon][noalloc]") {
  static struct minimon_cmd cmds[32];
  static char               names[RCSW_ARRAY_ELTS(cmds)][8];
  for (size_t i = 0; i < RCSW_ARRAY_ELTS(cmds); ++i) {
    snprintf(names[i], sizeof(names[i]), "c%zu", i);
    cmds[i]       = make_finish_cmd();
    cmds[i].name  = names[i];
    cmds[i].alias = nullptr;
  }
  struct minimon_config c;
  memset(&c, 0, sizeof(c));
  c.cmds            = cmds;
  c.n_cmds          = RCSW_ARRAY_ELTS(cmds);
  c.include_builtin = true;

  /* The last of 32 user commands, after the 6 built-ins */
  CATCH_REQUIRE(9 == run_monitor("c31 9\n", &c));
}
