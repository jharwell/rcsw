/**
 * \file
 *
 * \copyright 2023 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * \ingroup console
 *
 * \brief Minimon, an interactive serial monitor for board bring-up.
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/al/types.h"
#include "rcsw/core/compilers.h"

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
/** \brief Maximum number of arguments a command may accept. */
#define MINIMON_CMD_MAX_ARGS 4

/** \brief Maximum length of a command name string, including null terminator. */
#define MINIMON_CMD_MAX_NAMELEN 16

/** \brief The type of a command parameter. */
enum minimon_param_type {
  MINIMON_PARAM_UINT32, /**< A 32-bit unsigned number. */
  MINIMON_PARAM_STR,    /**< A string. */
};

/** \brief Which help text to print. */
enum minimon_help_type {
  MINIMON_HELP_SHORT, /**< One line per command. */
  MINIMON_HELP_LONG   /**< Including per-parameter help. */
};

/*******************************************************************************
 * Types
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Optional interrupt-related callbacks invoked around command
 * execution.
 *
 * If provided, \c mask_all is called before a command executes and \c
 * clear_all is called after.
 */
struct minimon_irq_callbacks {
  void (*clear_all)(void);  ///< Unmask interrupts after a command.
  void (*mask_all)(void);   ///< Mask interrupts before a command.
};

/**
 * \brief Character I/O callbacks for a stream.
 */
struct minimon_stream_callbacks {
  int (*putchar)(int c);  ///< Write one character.
  int (*getchar)(void);   ///< Read one character; negative if none is ready.
};

/**
 * \brief Initialization parameters for \ref minimon_init().
 */
struct minimon_config {
  /**
   * \brief Optional interrupt callbacks. May be zero-initialized if not
   * needed.
   */
  struct minimon_irq_callbacks irqcb;

  /**
   * \brief Optional stream1 callbacks for \c load and \c send commands.
   *
   * If both callbacks are set, stream0 is used exclusively for interactive
   * I/O and stream1 for data transfer. If they are NULL (zero-initialized),
   * both use stream0 and \c load / \c send suppress progress output.
   */
  struct minimon_stream_callbacks stream1;

  /**
   * \brief Array of custom commands to register, or NULL if none.
   *
   * Custom commands are registered in addition to (or instead of, if
   * \ref minimon_config.include_builtin is false) the built-in command set.
   * Nothing is copied: the monitor uses the array in place, so it and the
   * strings it points to must stay valid for as long as the monitor runs
   * (typically a \c static \c const table). There is no limit on the number
   * of commands.
   */
  const struct minimon_cmd* cmds;

  /**
   * \brief Number of entries in \ref minimon_config.cmds. Ignored if it is
   * NULL.
   */
  size_t n_cmds;

  /**
   * \brief If true, include the built-in commands (read, write, jump,
   * load, send). The HELP command is always included regardless.
   */
  bool include_builtin;

  /**
   * \brief If true, print the full help menu when \ref minimon_start()
   * is called. Useful to disable when there are many commands and you
   * want the startup output to remain concise.
   */
  bool help_on_start;
};

/**
 * \brief A single argument passed to a \ref minimon_cmd, i.e. what was actually
 * passed, as opposed to \ref minimon_cmd_param, which describes what is
 * expected.
 */
union minimon_cmd_arg {
  uint32_t num;  ///< For \ref MINIMON_PARAM_UINT32.
  char*    s;    ///< For \ref MINIMON_PARAM_STR.
};

/**
 * \brief Descriptor for one parameter of a \ref minimon_cmd.
 *
 * Defines what the command expects; contrast with \ref minimon_cmd_arg,
 * which holds what was actually passed.
 */
struct minimon_cmd_param {
  const char*             name;        ///< Parameter name, shown in help.
  enum minimon_param_type type;        ///< How the argument is parsed.
  const char*             short_help;  ///< One-line help.
  const char*             long_help;   ///< Detailed help.
  bool_t                  required;  ///< If false, \c dflt is used when omitted.
  union minimon_cmd_arg   dflt;      ///< Default for an optional parameter.
};

/**
 * \brief A command the monitor can execute: its name, help string, execution
 * function, etc.
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
   * arguments (in the order defined by \ref minimon_cmd.config). Retrieve
   * arguments with \c va_start() / \c va_arg(). The same hook may be registered
   * for multiple commands; use \p cmdname to distinguish them.
   */
  void (*hook)(const char* cmdname, ...);

  /**
   * \brief Array of parameter definitions for the command.
   */
  struct minimon_cmd_param config[MINIMON_CMD_MAX_ARGS];
};

/**
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
 * - **stream1** — optional; provided via \ref minimon_config.stream1. If
 *   supplied, the \c load and \c send commands use stream1 for data
 *   transfer and print progress to stream0. If omitted, \c load and \c
 *   send share stream0 and suppress progress output.
 *
 * \section minimon_custom_cmds Custom Commands
 *
 * To add commands beyond the built-ins, populate an array of \ref
 * minimon_cmd structs and pass it via \ref minimon_config.cmds and \ref
 * minimon_config.n_cmds to \ref minimon_init(). Set \ref
 * minimon_config.include_builtin to true to retain the standard command
 * set alongside your custom ones, or false to replace it entirely (the
 * HELP command is always included regardless).
 *
 * Each \ref minimon_cmd specifies a name, an optional short alias, a help
 * string, a \ref minimon_cmd.hook function pointer, and up to \ref
 * MINIMON_CMD_MAX_ARGS parameter descriptors. The hook receives the
 * command name and any parsed arguments via variadic arguments; retrieve
 * them with \c va_start() / \c va_arg().
 *
 * \note The struct itself is the monitor's internal state (one instance,
 * inside RCSW); applications configure it through \ref minimon_init().
 */
struct minimon {
  struct minimon_stream_callbacks stream0;
  struct minimon_stream_callbacks stream1;

  /** Built-ins in use: all of them, or just HELP. */
  size_t n_builtin_cmds;

  /** The caller's commands, used in place (\ref minimon_config.cmds). */
  const struct minimon_cmd* user_cmds;

  /** # entries in \c user_cmds. */
  size_t n_user_cmds;

  struct minimon_irq_callbacks irqcb;
  bool_t                       help_on_start;
};

/** \brief Pointer to a \c void(void) function, e.g. a \c jump target. */
typedef void (*vfp_t)(void);

/******************************************************************************
 * Public API
 *****************************************************************************/
/**
 * \brief Initialize the monitor.
 *
 * Must be called before \ref minimon_start(). Registers the command set,
 * IRQ callbacks, and stream callbacks. \p config may be NULL to use
 * defaults (built-in commands only, help on start, no IRQ callbacks, no
 * stream1).
 *
 * \param config Initialization parameters, or NULL for defaults.
 */
RCSW_API void minimon_init(const struct minimon_config* config);

/**
 * \brief Start the monitor loop.
 *
 * Reads commands from stream0, parses and validates their syntax, and
 * dispatches them to the appropriate hook. A command is matched by its exact
 * name or alias. Never returns, though control may be transferred elsewhere
 * via the \c jump command.
 *
 * Negative values from \ref stdio_getchar() (and from stream1's getchar
 * during \c load) are treated as "no character available yet" and ignored,
 * so a polling driver may return -1 when its receive buffer is empty.
 */
RCSW_API void minimon_start(void) RCSW_NORETURN;

END_C_DECLS
