/**
 * \file minimon.h
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
#define MINIMON_CMD_MAX_ARGS  4 /* max number of arguments to a command */
#define MINIMON_MAX_CMDS 20
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
 * \brief Optional interrupt-related callbacks for the monitor to be used when
 * running commands.
 */
struct minimon_irq_callbacks {
  void (*clear_all)(void);
  void (*mask_all)(void);
};

struct minimon_stream_callbacks {
  int (*putchar)(int c);
  int (*getchar)(void);
};

/**
 * \brief Parameters for initializing the monitor prior to starting it.
 *
 * If \ref minimon_params.stream1 callabacks are omitted, then the \c load and
 * \c send commands will use stream0 (same stream as user input/output) and not
 * print their progress to stream0. If \ref minimon_params.stream1 callbacks are
 * provided, then stream0 will be exclusively used for the user input/output,
 * and stream1 for the \c load and \c send commands.
 */
struct minimon_params {
  struct minimon_irq_callbacks irqcb;

  /**
   * \brief The callbacks for getting/sending a character to stream1 in minimon,
   * which is used for the \p load and \p send commands. Stream0 is used for user
   * input/output, and always uses stdio_putchar()/\ref stdio_getchar().
   */
  struct minimon_stream_callbacks stream1;
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
 * \brief Representation of a single parameter for a \ref minimon_cmd: type,
 * help string, is it required, etc.
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
   * \brief The main command hook function. When executed by the monitor, it is
   * passed the name of the cmd (in case you want to use the same hook for
   * multiple commands), and necessary arguments, which should be retrieved as
   * needed via \c va_start() and \c va_arg().
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
 * \brief Configure minimon before starting it by giving it a list of supported
 * cmds, callbacks for handling things like disabling interrupts, etc.
 *
 * \param params Initialization parameters. Can be \c NULL.
 */

RCSW_API void minimon_init(const struct minimon_params* params);

/**
 * \brief Handles reading in commands typed in the serial terminal,
 *        parsing/validating their syntax, and then calling the appropriate
 *        function to execute the command.
 *
 * Nominally does not return; may jump to another execution location if
 * commanded.
 */
RCSW_API void minimon_start(void) RCSW_DEAD;

END_C_DECLS
