/**
 * \file er.h
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
 #include "rcsw/common/common.h"

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
/* \cond INTERNAL */
#define LIBRA_ERL_NONE  0 /* No event reporting */
#define LIBRA_ERL_FATAL 1 /* Fatal events only */
#define LIBRA_ERL_ERROR 2 /* Fatal, error events only */
#define LIBRA_ERL_WARN  3 /* Fatal, error, warn events only */
#define LIBRA_ERL_INFO  4 /* Fatal, error, warn, info events only */
#define LIBRA_ERL_DEBUG 5 /* Fatal, error, warn, info, debug events only */
#define LIBRA_ERL_TRACE 6 /* All events */
#define LIBRA_ERL_ALL   LIBRA_ERL_TRACE

#define RCSW_ERL_NONE  LIBRA_ERL_NONE
#define RCSW_ERL_FATAL LIBRA_ERL_FATAL
#define RCSW_ERL_ERROR LIBRA_ERL_ERROR
#define RCSW_ERL_WARN  LIBRA_ERL_WARN
#define RCSW_ERL_INFO  LIBRA_ERL_INFO
#define RCSW_ERL_DEBUG LIBRA_ERL_DEBUG
#define RCSW_ERL_TRACE LIBRA_ERL_TRACE
#define RCSW_ERL_ALL   LIBRA_ERL_ALL

#if defined(LIBRA_ERL_INHERIT) && !defined(LIBRA_ERL)
#error LIBRA_ERL_INHERIT defined but LIBRA_ERL not defined!
#endif

/*
 * If rcsw is used in a context where this is not defined it is almost assuredly
 * an error, buuuttttt RCSW might need to compile in weird environments.
 */
#if !defined(LIBRA_ERL)
#define LIBRA_ERL LIBRA_ERL_ALL
#endif
/* \endcond */

/**
 * \brief The compile-time event reporting level.
 */
#define RCSW_ERL LIBRA_ERL

/**
 * \brief Debug color codes (for producing colored terminal output)
 */
#define RCSW_ER_HEADC "\033[36m" /* blue */
#define RCSW_ER_OKC "\033[32m"   /* green */
#define RCSW_ER_WARNC "\033[33m" /* yellow */
#define RCSW_ER_FAILC "\033[31m" /* red */
#define RCSW_ER_ENDC "\033[0m"   /* reset to default terminal text color */

/**
 * A lighter/simpler version of log4c. See docs for a detailed description.
 */
#define RCSW_ER_PLUGIN_LOG4CL 0

/**
 * The simple bare-bones logger. See docs for a detailed description.
 */
#define RCSW_ER_PLUGIN_SIMPLE 1

/**
 * Specify that all logging go to the zlog framework.
 *
 * This is the most full-featured of RCSW's built-in logging schemes, supporting
 * everything zlog does.
 *
 * This is useful in:
 *
 * - Linux targets and targets with a full-featured OS.
 */
#define RCSW_ER_PLUGIN_ZLOG 2

/**
 * Specify that all logging go to a custom reporting macro.
 */
#define RCSW_ER_PLUGIN_CUSTOM 3

/*******************************************************************************
 * Macros
 ******************************************************************************/
#define RCSW_ER_MODNAME_BUILDER_IMPL(X) X RCSW_ER_PLUGIN_MODNAME_COMPONENT_SEPARATOR

/**
 * \def RCSW_ER_MODNAME_BUILDER(...) Define the name of a logging module
 *
 * Takes a comma-separated list of string components of the name you want your
 * module to have. Each "component" corresponds to a level in hierarchical
 * logging scheme. For example, if you want to create a module called "mymodule"
 * in a project called "myproject", you would do:
 *
 * \code
 * #define RCSW_ER_MODNAME RCSW_ER_MODNAME_BUILDER("myproject", "mymodule")
 * \endcode
 *
 * which would be expanded as appropriate ("myproject.mymodule",
 * "myproject_mymodule", etc.) depending on the plugin you build RCSW with.
 *
 * This macro ensures that you can use the same code with multple ER plugins.
 */
#define RCSW_ER_MODNAME_BUILDER(...)                                    \
  RCSW_XFOR_EACH1_NOTAIL(RCSW_ER_MODNAME_BUILDER_IMPL, __VA_ARGS__)
