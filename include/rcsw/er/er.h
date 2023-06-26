/**
 * \file er.hpp
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
/*
 * If rcsw is used in a context where this is not defined it is almost
 * assuredly an error.
 */
#if !defined(LIBRA_ER)
#error LIBRA_ER not defined--defaulting to ER_NONE
#endif

#if defined(LIBRA_ER_INHERIT) && !defined(LIBRA_ER)
#error LIBRA_ER_INHERIT but LIBRA_ER not defined
#endif

#define RCSW_ER LIBRA_ER

#define LIBRA_ER_NONE  0 /* No event reporting */
#define LIBRA_ER_FATAL 1 /* Fatal events only */
#define LIBRA_ER_ALL   2 /* All event reporting  */

#define RCSW_ER_NONE  LIBRA_ER_NONE
#define RCSW_ER_FATAL LIBRA_ER_FATAL
#define RCSW_ER_ALL   LIBRA_ER_ALL

/**
 * \brief Debug color codes (for producing colored terminal output)
 */
#define RCSW_ER_HEADC "\033[36m" /* blue */
#define RCSW_ER_OKC "\033[32m"   /* green */
#define RCSW_ER_WARNC "\033[33m" /* yellow */
#define RCSW_ER_FAILC "\033[31m" /* red */
#define RCSW_ER_ENDC "\033[0m"   /* reset to default terminal text color */

/**
 * The simple bare-bones logger.
 *
 * This plugin uses RCSW's built-in printf() and minimal stdlib implementation,
 * to provide the necessary logging functionality in environments where stdlib
 * is not available, and/or using stdlib hogs too much space.
 *
 * This plugin supports the following logging levels, ordered from highest to
 * lowest priority: FATAL, ERROR, INFO, WARN, DEBUG, TRACE.
 *
 * In this plugin, each source file within RCSW and of each project which links
 * with RCSW can define a logging "module"; modules are file-based, and
 * therefore you can't have multiple modules/loggers in a single file. If you
 * architect your projects well, this should not be a burdensome restriction.
 * The modules in this plugin:
 *
 * - Are unconditionally enabled and cannot be disabled; that is, each time a
 *   logging statement is encountered during execution, it is always emitted.
 *
 * - Do not have a "level" associated with them which determines if a given
 *   logging statement should be emitted. Put another way, the implicit level of
 *   all modules in this plugin is TRACE.
 *
 * - The name of each module is prepended to each logging statement to help
 *   identify the logging source.
 *
 * Each emitted logging statement is of the form:
 *
 * <RCSW_ER_MODULE_NAME> [LVL] <message>
 *
 * Where \ref RCSW_ER_MODULE_NAME is the #define defining the name of the
 * module. If it is omitted, __FILE_NAME__ is used. \a LVL is one of FATAL,
 * ERROR, INFO, WARN, DEBUG, TRACE.
 *
 * This plugin is useful in:
 *
 * - Bare metal environments such as bootstraps without an OS.
 *
 * - Bare metal hardware validation tests.
 */
#define RCSW_ER_PLUGIN_SIMPLE 1

/**
 * A lighter/simpler version of log4c.
 *
 * This plugin supports the following logging levels, ordered from highest to
 * lowest priority: FATAL ERROR, INFO, WARN, DEBUG, TRACE.

 * In this plugin, each source file within RCSW and of each project which links
 * with RCSW can define a logging "module"; modules are file-based, and
 * therefore you can't have multiple modules/loggers in a single file. If you
 * architect your projects well, this should not be a burdensome restriction.
 *
 * Each module can be be enabled/disabled independently in a lightweight
 * manner; unlike log4j loggers, the enable/disable status of one logger/module
 * does not have any effect on the status of another. That is, the hierarchy is
 * "flat".
 *
 * This plugin does not provide most features found in log4c, with the exception
 * of levels: INFO, WARN, DEBUG, TRACE, FATAL. If you want features from log4c,
 * use the log4c plugin.
 *
 * Each emitted logging statement is of the form:
 *
 * <RCSW_ER_MODULE_NAME> [LVL] <message>
 *
 * Where \ref RCSW_ER_MODULE_NAME is the #define defining the name of the
 * module. If it is omitted, __FILE_NAME__ is used. \a LVL is one of FATAL,
 * ERROR, INFO, WARN, DEBUG, TRACE.
 *
 * This is useful in:
 *
 * - Medium-complexity embedded systems with limited resources
 *
 * - Systems where you want to have multiple modules, only some of which should
 *   be enabled, but don't need the hierarchical logging of log4c. In this
 *   scheme, modules are either enabled or not whether a given module is enabled
 *   has no effect on other modules. In addition, the name given to a specific
 *   module is purely for debugging purposes, and has no effect on event
 *   reporting; you can have multiple modules with the same name and different
 *   IDs, if you want.
 */
#define RCSW_ER_PLUGIN_LOG4CL 0

/**
 * Specify that all logginggo to the log4c framework.
 *
 * This is the most full-featured of RCSW's built-in logging schemes, supporting
 * everything log4c does.
 *
 * This is useful in:
 *
 * - Linux targets and targets with a full-featured OS.
 */
#define RCSW_ER_PLUGIN_LOG4C 2

/**
 * Specify that all logging go to a custom reporting macro.
 */
#define RCSW_ER_PLUGIN_CUSTOM 3

/*
 * By default, assume the simple logger, since we don't know if log4c exists on
 * the systems.
 */
#if !defined(LIBRA_ER_PLUGIN)
#define LIBRA_ER_PLUGIN_SIMPLE
#endif
