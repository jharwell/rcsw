/**
 * \file
 *
 * \copyright 2023 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * \ingroup er
 *
 * \brief Event reporting levels, plugin IDs, and module name construction.
 */

#pragma once

#include "rcsw/core/variadics.h"

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
/* \cond INTERNAL */
#define LIBRA_ERL_NONE 0  /* No event reporting */
#define LIBRA_ERL_FATAL 1 /* Fatal events only */
#define LIBRA_ERL_ERROR 2 /* Fatal, error events only */
#define LIBRA_ERL_WARN 3  /* Fatal, error, warn events only */
#define LIBRA_ERL_INFO 4  /* Fatal, error, warn, info events only */
#define LIBRA_ERL_DEBUG 5 /* Fatal, error, warn, info, debug events only */
#define LIBRA_ERL_TRACE 6 /* All events */
#define LIBRA_ERL_ALL LIBRA_ERL_TRACE
/* \endcond */

/**
 * \name Event reporting levels
 *
 * Values for \ref RCSW_ERL and for run-time module levels (e.g.
 * \ref log4cl_mod_lvl_set()). See
 * \rcswdoc{concepts/event-reporting/levels}.
 * @{
 */
#define RCSW_ERL_NONE LIBRA_ERL_NONE   /**< No event reporting. */
#define RCSW_ERL_FATAL LIBRA_ERL_FATAL /**< FATAL only. */
#define RCSW_ERL_ERROR LIBRA_ERL_ERROR /**< FATAL and ERROR. */
#define RCSW_ERL_WARN LIBRA_ERL_WARN   /**< FATAL through WARN. */
#define RCSW_ERL_INFO LIBRA_ERL_INFO   /**< FATAL through INFO. */
#define RCSW_ERL_DEBUG LIBRA_ERL_DEBUG /**< FATAL through DEBUG. */
#define RCSW_ERL_TRACE LIBRA_ERL_TRACE /**< Everything. */
#define RCSW_ERL_ALL LIBRA_ERL_ALL     /**< Same as \ref RCSW_ERL_TRACE. */
/** @} */

/* \cond INTERNAL */
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
 * \name Terminal color codes
 *
 * ANSI escape sequences for colored terminal output.
 * @{
 */
#define RCSW_ER_HEADC "\033[36m" /**< Cyan. */
#define RCSW_ER_OKC "\033[32m"   /**< Green. */
#define RCSW_ER_WARNC "\033[33m" /**< Yellow. */
#define RCSW_ER_FAILC "\033[31m" /**< Red. */
#define RCSW_ER_ENDC "\033[0m"   /**< Reset to the default text color. */
/** @} */

/**
 * \name ER plugin IDs
 *
 * Values for \c RCSW_CONFIG_ER_PLUGIN. Any other value selects a custom plugin.
 * See \rcswdoc{library/er}.
 * @{
 */

/** \brief LOG4CL: a lighter, simpler log4c with per-module levels. */
#define RCSW_ER_PLUGIN_LOG4CL 0

/** \brief The simple plugin: one build-wide level, no per-module control. */
#define RCSW_ER_PLUGIN_SIMPLE 1

/**
 * \brief zlog: the most full-featured built-in plugin. For Linux and other
 * targets with a full-featured OS.
 */
#define RCSW_ER_PLUGIN_ZLOG 2

/** @} */

/*******************************************************************************
 * Macros
 ******************************************************************************/
/* \cond INTERNAL */
#define RCSW_ER_MODNAME_BUILDER_IMPL(X) \
  X RCSW_ER_PLUGIN_MODNAME_COMPONENT_SEPARATOR
/* \endcond */

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
 * This macro ensures that you can use the same code with multiple ER plugins.
 */
#define RCSW_ER_MODNAME_BUILDER(...) \
  RCSW_XFOR_EACH1_NOTAIL(RCSW_ER_MODNAME_BUILDER_IMPL, __VA_ARGS__)
