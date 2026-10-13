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
#include <cerrno>
#include <csetjmp>
#include <cstdio>
#include <cstring>
#include <functional>
#include <string>
#include <vector>

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <catch2/catch_test_macros.hpp>

#include "rcsw/core/core.h"
#include "rcsw/console/minimon.h"
#include "rcsw/utils/checksum.h"

/*******************************************************************************
 * Helpers
 ******************************************************************************/
namespace {
std::jmp_buf g_done;
uint32_t     g_arg;

void finish_hook(const char*, const union minimon_cmd_arg* args) {
  g_arg = args[0].num;
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
/*
 * A read/write file to capture stdout into. On Linux this is an anonymous
 * memfd, so it works where the filesystem (or /tmp) is read-only, as in some
 * CI containers; tmpfile() is the fallback.
 */
FILE* open_capture() {
#if defined(__linux__) && defined(MFD_CLOEXEC)
  int fd = memfd_create("minimon-capture", MFD_CLOEXEC);
  if (fd >= 0) {
    FILE* f = fdopen(fd, "w+");
    if (nullptr != f) {
      return f;
    }
    close(fd);
  }
#endif
  return tmpfile();
}

uint32_t run_monitor(const char*                  input,
                     const struct minimon_config* config     = nullptr,
                     const std::function<void()>& after_init = {},
                     std::string*                 output     = nullptr) {
  int fds[2];
  CATCH_REQUIRE(0 == pipe(fds));
  CATCH_REQUIRE(static_cast<ssize_t>(strlen(input)) ==
                write(fds[1], input, strlen(input)));
  close(fds[1]);

  fflush(stdout);
  int   saved_in  = dup(STDIN_FILENO);
  int   saved_out = dup(STDOUT_FILENO);
  FILE* capture   = (nullptr != output) ? open_capture() : nullptr;
  if (nullptr != output) {
    /* Fail loudly: without a capture file the output would just be lost */
    CATCH_INFO("could not create an output capture file: " << strerror(errno));
    CATCH_REQUIRE(nullptr != capture);
  }
  int   sink = (nullptr != capture) ? fileno(capture)
                                    : open("/dev/null", O_WRONLY);
  dup2(fds[0], STDIN_FILENO);
  dup2(sink, STDOUT_FILENO);
  close(fds[0]);
  if (nullptr == capture) {
    close(sink);
  }

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
  if (nullptr != capture) {
    output->clear();
    rewind(capture);
    char   buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), capture)) > 0) {
      output->append(buf, n);
    }
    fclose(capture);
  }
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

namespace {
/* A command with mixed parameter types; the hook records its arguments. */
uint32_t    g_mixed_a;
std::string g_mixed_b;
uint32_t    g_mixed_c;

void mixed_hook(const char*, const union minimon_cmd_arg* args) {
  g_mixed_a = args[0].num;
  g_mixed_b = args[1].s;
  g_mixed_c = args[2].num;
}

struct minimon_cmd make_mixed_cmd() {
  struct minimon_cmd cmd;
  memset(&cmd, 0, sizeof(cmd));
  cmd.name                 = "mixed";
  cmd.alias                = "m";
  cmd.help                 = "Mixed parameter types";
  cmd.hook                 = mixed_hook;
  cmd.config[0].name       = "a";
  cmd.config[0].type       = MINIMON_PARAM_UINT32;
  cmd.config[0].short_help = "number";
  cmd.config[0].long_help  = "number";
  cmd.config[0].required   = true;
  cmd.config[1].name       = "b";
  cmd.config[1].type       = MINIMON_PARAM_STR;
  cmd.config[1].short_help = "string";
  cmd.config[1].long_help  = "string";
  cmd.config[1].required   = true;
  cmd.config[2].name       = "c";
  cmd.config[2].type       = MINIMON_PARAM_UINT32;
  cmd.config[2].short_help = "optional number";
  cmd.config[2].long_help  = "optional number";
  cmd.config[2].required   = false;
  cmd.config[2].dflt.num   = 77;
  return cmd;
}

/* Stream1: output captured in a string, input served from a string. */
std::string g_s1_out;
std::string g_s1_in;
size_t      g_s1_pos;

int s1_putchar(int c) {
  g_s1_out.push_back(static_cast<char>(c));
  return c;
}
int s1_getchar(void) {
  return g_s1_pos < g_s1_in.size()
           ? static_cast<unsigned char>(g_s1_in[g_s1_pos++])
           : -1;
}

/* The built-ins plus "finish", optionally with stream1 attached. */
struct minimon_config builtin_config(bool with_stream1) {
  static struct minimon_cmd cmds[1];
  cmds[0] = make_finish_cmd();
  struct minimon_config c;
  memset(&c, 0, sizeof(c));
  c.cmds            = cmds;
  c.n_cmds          = 1;
  c.include_builtin = true;
  if (with_stream1) {
    c.stream1.putchar = s1_putchar;
    c.stream1.getchar = s1_getchar;
  }
  g_s1_out.clear();
  g_s1_in.clear();
  g_s1_pos = 0;
  return c;
}

/*
 * Memory the monitor can address: minimon addresses are 32-bit numbers, so on
 * a 64-bit host the memory must be mapped below 4 GiB.
 */
uint32_t* low_mem() {
  static uint32_t* mem = nullptr;
  if (nullptr == mem) {
#if defined(MAP_32BIT)
    void* p = mmap(nullptr,
                   4096,
                   PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT,
                   -1,
                   0);
    if (MAP_FAILED != p) {
      mem = static_cast<uint32_t*>(p);
    }
#else
    if (sizeof(uintptr_t) == sizeof(uint32_t)) {
      static uint32_t buf[1024];
      mem = buf;
    }
#endif
  }
  return mem;
}

std::string addr_of(const void* p) {
  char buf[16];
  snprintf(buf,
           sizeof(buf),
           "0x%08lx",
           static_cast<unsigned long>(reinterpret_cast<uintptr_t>(p)));
  return buf;
}
} /* namespace */

CATCH_TEST_CASE("minimon rejects malformed and oversized numbers",
                "[tool][minimon][noalloc]") {
  /* "12AB" is not decimal and 2^32 does not fit; neither is truncated */
  CATCH_REQUIRE(31 == run_monitor("finish 12AB\n"
                                  "finish 4294967296\n"
                                  "finish 0x1F\n"));
  CATCH_REQUIRE(0xFFFFFFFFU == run_monitor("finish 4294967295\n"));
}

CATCH_TEST_CASE("minimon ignores leading whitespace on a command line",
                "[tool][minimon][noalloc]") {
  CATCH_REQUIRE(6 == run_monitor("   finish 6\n"));
}

CATCH_TEST_CASE("minimon passes hook arguments with their declared types",
                "[tool][minimon][noalloc]") {
  static struct minimon_cmd cmds[2];
  cmds[0] = make_mixed_cmd();
  cmds[1] = make_finish_cmd();
  struct minimon_config c;
  memset(&c, 0, sizeof(c));
  c.cmds            = cmds;
  c.n_cmds          = 2;
  c.include_builtin = true;

  /* the omitted optional argument gets its default */
  CATCH_REQUIRE(1 == run_monitor("mixed 0x10 hello\nfinish 1\n", &c));
  CATCH_REQUIRE(0x10 == g_mixed_a);
  CATCH_REQUIRE("hello" == g_mixed_b);
  CATCH_REQUIRE(77 == g_mixed_c);

  CATCH_REQUIRE(2 == run_monitor("m 4294967295 x 5\nfinish 2\n", &c));
  CATCH_REQUIRE(0xFFFFFFFFU == g_mixed_a);
  CATCH_REQUIRE("x" == g_mixed_b);
  CATCH_REQUIRE(5 == g_mixed_c);
}

CATCH_TEST_CASE("minimon write and read access memory",
                "[tool][minimon][noalloc]") {
  uint32_t* mem = low_mem();
  if (nullptr == mem) {
    CATCH_SKIP("no memory with a 32-bit address on this host");
  }
  mem[2]                = 0x12345678U;
  struct minimon_config c = builtin_config(false);
  std::string           out;
  std::string           in = "w " + addr_of(mem) + " 0xDEADBEEF 2\n" + "read " +
                   addr_of(mem) + " 3\n" + "finish 1\n";
  CATCH_REQUIRE(1 == run_monitor(in.c_str(), &c, {}, &out));
  CATCH_INFO("captured output: [" << out << "]");
  CATCH_REQUIRE(0xDEADBEEFU == mem[0]);
  CATCH_REQUIRE(0xDEADBEEFU == mem[1]);
  CATCH_REQUIRE(0x12345678U == mem[2]);
  CATCH_REQUIRE(out.find("0xDEADBEEF 0xDEADBEEF 0x12345678") !=
                std::string::npos);
}

CATCH_TEST_CASE("minimon send raw on stream0 sends exactly SIZE bytes",
                "[tool][minimon][noalloc]") {
  uint32_t* mem = low_mem();
  if (nullptr == mem) {
    CATCH_SKIP("no memory with a 32-bit address on this host");
  }
  memcpy(mem, "ABCDEFGHIJKL", 12);
  struct minimon_config c = builtin_config(false);
  std::string           out;
  std::string in = "send " + addr_of(mem) + " 6 RAW\nfinish 1\n";
  CATCH_REQUIRE(1 == run_monitor(in.c_str(), &c, {}, &out));
  CATCH_INFO("mem at " << addr_of(mem) << ", captured output: [" << out << "]");
  CATCH_REQUIRE(out.find("ABCDEF") != std::string::npos);
  CATCH_REQUIRE(out.find("ABCDEFG") == std::string::npos);
  /* no progress text mixed into the data */
  CATCH_REQUIRE(out.find("Sending") == std::string::npos);
}

CATCH_TEST_CASE("minimon send and load use stream1, with progress on stream0",
                "[tool][minimon][noalloc]") {
  uint32_t* mem = low_mem();
  if (nullptr == mem) {
    CATCH_SKIP("no memory with a 32-bit address on this host");
  }
  memcpy(mem, "ABCDEFGHIJKL", 12);
  struct minimon_config c = builtin_config(true);
  std::string           out;
  std::string in = "send " + addr_of(mem) + " 6 raw\nfinish 1\n";
  CATCH_REQUIRE(1 == run_monitor(in.c_str(), &c, {}, &out));
  CATCH_INFO("captured output: [" << out << "]");
  CATCH_REQUIRE("ABCDEF" == g_s1_out);
  CATCH_REQUIRE(out.find("Sending 6 bytes") != std::string::npos);

  c        = builtin_config(true);
  g_s1_in  = std::string("\x01\x02\x03\x04\x05\x06", 6);
  mem[0]   = 0xFFFFFFFFU;
  mem[1]   = 0xFFFFFFFFU;
  in       = "load " + addr_of(mem) + " 6\nfinish 1\n";
  CATCH_REQUIRE(1 == run_monitor(in.c_str(), &c, {}, &out));
  auto* bytes = reinterpret_cast<const uint8_t*>(mem);
  for (int i = 0; i < 6; ++i) {
    CATCH_REQUIRE(i + 1 == bytes[i]);
  }
  CATCH_REQUIRE(out.find("Receiving 6 bytes") != std::string::npos);
}

CATCH_TEST_CASE("minimon send NMEA checksums each packet on its own",
                "[tool][minimon][noalloc]") {
  uint32_t* mem = low_mem();
  if (nullptr == mem) {
    CATCH_SKIP("no memory with a 32-bit address on this host");
  }
  /* 18 words: one full packet of 16 and a partial one of 2 */
  for (uint32_t i = 0; i < 18; ++i) {
    mem[i] = i;
  }
  mem[17]                 = 0xFFFFFFFFU; /* printed unsigned */
  struct minimon_config c = builtin_config(true);
  std::string in = "send " + addr_of(mem) + " 70 NMEA\nfinish 1\n";
  CATCH_REQUIRE(1 == run_monitor(in.c_str(), &c));

  std::string body1 = "SEND,0";
  for (int i = 0; i < 16; ++i) {
    body1 += "," + std::to_string(i);
  }
  std::string body2  = "SEND,1,16,4294967295";
  auto        packet = [](const std::string& body) {
    char cks[8];
    snprintf(cks,
             sizeof(cks),
             "*%02X\r\n",
             utils_xchks8(reinterpret_cast<const uint8_t*>(body.data()),
                          body.size(),
                          0));
    return "$" + body + cks;
  };
  CATCH_REQUIRE(packet(body1) + packet(body2) == g_s1_out);
}

CATCH_TEST_CASE("minimon send rejects an unknown protocol",
                "[tool][minimon][noalloc]") {
  uint32_t* mem = low_mem();
  if (nullptr == mem) {
    CATCH_SKIP("no memory with a 32-bit address on this host");
  }
  struct minimon_config c = builtin_config(true);
  /* matched in full: "rawx" is not "raw" */
  std::string in = "send " + addr_of(mem) + " 4 rawx\nfinish 1\n";
  CATCH_REQUIRE(1 == run_monitor(in.c_str(), &c));
  CATCH_REQUIRE(g_s1_out.empty());
}
