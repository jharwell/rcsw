/**
 * \file minimon.h
 * \brief Bare-metal interactive monitor for board bring-up and hardware
 * validation.
 *
 * Minimon reads commands from a serial stream (stream0), executes them,
 * and prints results back. It has no dynamic memory requirements and no
 * OS dependencies, making it suitable for use before an RTOS or memory
 * allocator is initialized.
 *
 * \section minimon_streams Streams
 *
 * Minimon uses up to two streams:
 * - **stream0** — always \ref stdio_putchar() / \ref stdio_getchar(). Used
 *   for interactive user input and output.
 * - **stream1** — optional; provided via \ref minimon_params.stream1. If
 *   supplied, the \c load and \c send commands use stream1 for data
 *   transfer and print progress to stream0. If omitted, \c load and \c
 *   send share stream0 and suppress progress output.
 *
 * \section minimon_custom_cmds Custom Commands
 *
 * To add commands beyond the built-ins, populate an array of \ref
 * minimon_cmd structs and pass it via \ref minimon_params.cmds and \ref
 * minimon_params.n_cmds to \ref minimon_init(). Set \ref
 * minimon_params.include_builtin to TRUE to retain the standard command
 * set alongside your custom ones, or FALSE to replace it entirely (the
 * HELP command is always included regardless).
 *
 * Each \ref minimon_cmd specifies a name, an optional short alias, a help
 * string, a \ref minimon_cmd.hook function pointer, and up to \ref
 * MINIMON_CMD_MAX_ARGS parameter descriptors. The hook receives the
 * command name and any parsed arguments via variadic arguments; retrieve
 * them with \c va_start() / \c va_arg().
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/rcsw.h"

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
/** \brief Maximum number of arguments a command may accept. */
#define MINIMON_CMD_MAX_ARGS  4

/** \brief Maximum number of commands (built-in + custom) minimon supports. */
#define MINIMON_MAX_CMDS 20

/** \brief Maximum length of a command name string, including null terminator. */
#define MINIMON_CMD_MAX_NAMELEN 5

enum minimon_param_type {
  ekMINIMON_PARAM_UINT32,
  ekMINIMON_PARAM_STR,
};

enum minimon_help_type {
  ekMINIMON_HELP_SHORT,
  ekMINIMON_HELP_LONG
};

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/
/**
 * \brief Optional interrupt-related callbacks invoked around command
 * execution.
 *
 * If provided, \c mask_all is called before a command executes and \c
 * clear_all is called after.
 */
struct minimon_irq_callbacks {
  void (*clear_all)(void);
  void (*mask_all)(void);
};

/**
 * \brief Character I/O callbacks for a stream.
 */
struct minimon_stream_callbacks {
  int (*putchar)(int c);
  int (*getchar)(void);
};

/**
 * \brief Initialization parameters for \ref minimon_init().
 */
struct minimon_params {
  /**
   * \brief Optional interrupt callbacks. May be zero-initialized if not
   * needed.
   */
  struct minimon_irq_callbacks irqcb;

  /**
   * \brief Optional stream1 callbacks for \c load and \c send commands.
   *
   * If non-NULL, stream0 is used exclusively for interactive I/O and
   * stream1 is used for data transfer. If NULL, both use stream0 and
   * \c load / \c send suppress progress output.
   */
  struct minimon_stream_callbacks stream1;

  /**
   * \brief Array of custom commands to register, or NULL if none.
   *
   * Custom commands are registered in addition to (or instead of, if
   * \ref include_builtin is FALSE) the built-in command set. The array
   * must remain valid for the lifetime of the monitor.
   */
  struct minimon_cmd* cmds;

  /**
   * \brief Number of entries in \ref cmds. Ignored if \ref cmds is NULL.
   */
  size_t n_cmds;

  /**
   * \brief If TRUE, include the built-in commands (read, write, jump,
   * load, send). The HELP command is always included regardless.
   */
  bool include_builtin;

  /**
   * \brief If TRUE, print the full help menu when \ref minimon_start()
   * is called. Useful to disable when there are many commands and you
   * want the startup output to remain concise.
   */
  bool help_on_start;
};

/**
 * \brief Representation of a single argument passed to a \ref minimon_cmd. Note
 * that this is different than \ref minimon_cmd_param: that defines the
 * characteristics of WHAT needs to be passed; this defines what ACTUALLY is
 * passed.
 */
union minimon_cmd_arg {
  uint32_t num;
  char* s;
};

/**
 * \brief Descriptor for one parameter of a \ref minimon_cmd.
 *
 * Defines what the command expects; contrast with \ref minimon_cmd_arg,
 * which holds what was actually passed.
 */
struct minimon_cmd_param {
  const char* name;
  enum minimon_param_type type;
  const char* short_help;
  const char* long_help;
  bool_t required;
  union minimon_cmd_arg dflt;
};

/**
 * Representation of a single command executable in the monitor: its name, help
 * string, execution function, etc.
 */
struct minimon_cmd {
  /**
   * \brief The name of the command.
   */
  const char* name;

  /**
   * \brief A short form alias of the command. Must be a substring of \ref
   * minimon_cmd.name
   */
  const char* alias;

  /**
   * \brief The help string for the command. Should not terminate with a period,
   * and consist only of a single sentence.
   */
  const char* help;

  /**
   * \brief Command implementation function.
   *
   * Called by the monitor with the matched command name and any parsed
   * arguments (in the order defined by \ref params). Retrieve arguments
   * with \c va_start() / \c va_arg(). The same hook may be registered
   * for multiple commands; use \p cmdname to distinguish them.
   */
  void (*hook)(const char* cmdname, ...);

  /**
   * \brief Array of parameter definitions for the command.
   */
  struct minimon_cmd_param params[MINIMON_CMD_MAX_ARGS];
};

/**
 * \brief Monitor state structure.
 */
struct minimon {
  struct minimon_stream_callbacks stream0;
  struct minimon_stream_callbacks stream1;
  struct minimon_cmd cmds[MINIMON_MAX_CMDS];
  struct minimon_irq_callbacks irqcb;
  bool_t help_on_start;
};

/*******************************************************************************
 * Type Definitions
 ******************************************************************************/
typedef void (*vfp_t)(void);

/******************************************************************************
 * API Functions
 *****************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Initialize the monitor.
 *
 * Must be called before \ref minimon_start(). Registers the command set,
 * IRQ callbacks, and stream callbacks. \p params may be NULL to use
 * defaults (built-in commands only, help on start, no IRQ callbacks, no
 * stream1).
 *
 * \param params Initialization parameters, or NULL for defaults.
 */

RCSW_API void minimon_init(const struct minimon_params* params);

/**
 * \brief Start the monitor loop.
 *
 * Reads commands from stream0, parses and validates their syntax, and
 * dispatches them to the appropriate hook. Does not return under normal
 * operation; may transfer control to another address if the \c jump
 * command is executed.
 */
RCSW_API void minimon_start(void) RCSW_DEAD;

END_C_DECLS
