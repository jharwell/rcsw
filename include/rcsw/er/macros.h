/**
 * \file macros.h
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/er/er.h"
#include "rcsw/rcsw.h"

/*******************************************************************************
 * Macros when ER is enabled for FATAL events:
 *
 * - No plugin is used. When reporting FATAL events only, this is
 *   frequently in production and/or using a full-featured ER plugin is too
 *   costly in terms of execution time or space.
 *
 * - Debug printing macros enabled.
 ******************************************************************************/

#if (RCSW_ERL >= RCSW_ERL_FATAL)

/**
 * \brief Initialize the defined ER plugin.
 *
 * All usage of any ER machinery is undefined until this call.
 *
 * \note May not be idempotent if the underlying plugin initialization function
 * is not idempotent. See plugin documentation for details.
 */
#define RCSW_ER_INIT(...) RCSW_ER_PLUGIN_INIT(__VA_ARGS__)

/**
 * \brief Uninitialize/shutdown the defined ER plugin.
 *
 * All usage of any ER machinery is undefined after this call until \ref
 * RCSW_ER_INIT is called.
 *
 * \note May not be idempotent if the underlying plugin shutdown function
 * is not idempotent. See plugin documentation for details.
 */
#define RCSW_ER_DEINIT(...) RCSW_ER_PLUGIN_DEINIT(__VA_ARGS__)

/**
 * \brief General debug macros that will display whenever logging is enabled,
 * any other settings.
 */
#define DPRINTF(...) PRINTF(__VA_ARGS__)

/**
 * \brief Print a token AND it's value in decimal/hexadecimal.
 */
#define DPRINT_TOK(tok) DPRINTF(STR(tok) ": %d/0x%x\n", (int)(tok), (int)(tok));

/**
 * \brief Print a token AND it's value in decimal.
 */
#define DPRINT_TOKD(tok) DPRINTF(STR(tok) ": %d\n", (int)(tok));

/**
 * \brief Print a token AND it's value in hexadecimal.
 */
#define DPRINT_TOKX(tok) DPRINTF(STR(tok) ": 0x%x\n", (int)(tok));

/**
 * \brief Print a token AND it's value in floating point.
 */
#define DPRINT_TOKF(tok) DPRINTF(STR(tok) ": %.8f\n", (float)(tok));

#endif /* RCSW_ERL >= RCSW_ERL_FATAL */

#if (RCSW_ERL == RCSW_ERL_FATAL)

/**
 * \brief Emit a FATAL message. Does NOT use the ER plugin; uses \ref DPRINTF().
 */
#define ER_FATAL(msg, ...)                                              \
    {                                                                   \
      DPRINTF(RCSW_ER_MODNAME " [FATAL]: " msg, ## __VA_ARGS__);        \
    }                                                                   \

#define RCSW_ER_MODULE_INIT(...)

#elif (RCSW_ERL > RCSW_ERL_FATAL)

#define ER_FATAL(...)  ER_FATAL_IMPL(RCSW_ER_PLUGIN_HANDLE(RCSW_ER_MODID,   \
                                                           RCSW_ER_MODNAME), \
                                 __VA_ARGS__)

/* \cond INTERNAL */
#define ER_FATAL_IMPL(handle, ...) {                                    \
    if (RCSW_ER_PLUGIN_LVL_CHECK(handle, RCSW_ERL_FATAL)) {             \
      ER_REPORT(FATAL, handle, __VA_ARGS__)                             \
    }                                                                   \
  }
/* \endcond */
#endif

/*******************************************************************************
 * Macros when ER is enabled for severity level >= ERROR:
 *
 * - The configured plugin is used. Only [FATAL,ERROR] events are compiled in;
 *   others are discarded.
 *
 * - Debug printing macros enabled.
 ******************************************************************************/
#if (RCSW_ERL >= RCSW_ERL_ERROR)
/**
 * \def ER_ERR(...)
 *
 * Report a non-FATAL ERROR message.
 */
#define ER_ERR(...)  ER_ERR_IMPL(RCSW_ER_PLUGIN_HANDLE(RCSW_ER_MODID,   \
                                                       RCSW_ER_MODNAME), \
                                 __VA_ARGS__)

/* \cond INTERNAL */
#define ER_ERR_IMPL(handle, ...) {                                      \
    if (RCSW_ER_PLUGIN_LVL_CHECK(handle, ERROR)) {       \
      ER_REPORT(ERROR, handle, __VA_ARGS__)                             \
    }                                                                   \
  }
/* \endcond */

/**
 * \brief The name of an ER module. Used by some logging plugins to identify a
 * module.
 *
 * If not specified, defined as \a __FILE_NAME__.
 */
#if !defined(RCSW_ER_MODNAME)
#define RCSW_ER_MODNAME __FILE_NAME__
#endif

/**
 * \brief The ID of an ER module. Use by some logging plugins to identify a
 * module.
 *
 * If not specified, defined as -1.
 */
#if !defined(RCSW_ER_MODID)
#define RCSW_ER_MODID (-1)
#endif

/**
 * \def RCSW_ER_MODULE_INIT()
 *
 * Initialize a module in the currently selected ER plugin using the \ref
 * RCSW_ER_MODID and \ref RCSW_ER_MODNAME currently in scope.
 *
 * Initialization is idempotent if the selected plugin supports it.
 */
#define RCSW_ER_MODULE_INIT(...)                        \
  RCSW_ER_PLUGIN_INSMOD(RCSW_ER_MODID, RCSW_ER_MODNAME)

/**
 * \brief Install/enable an event report module in the current plugin.
 */

#define RCSW_ER_INSMOD(ID, NAME) RCSW_ER_PLUGIN_INSMOD(ID, NAME)

#endif /* RCSW_ERL >= RCSW_ERL_ERROR */

/*******************************************************************************
 * Macros when ER is enabled for severity level >= WARN:
 *
 * - The configured plugin is used. Only [FATAL,ERROR,WARN] events are compiled
 *   in; others are discarded.
 *
 * - Debug printing macros enabled.
 ******************************************************************************/
#if (RCSW_ERL >= RCSW_ERL_WARN)

/**
 * \def ER_WARN(...)
 *
 * Report a WARNING message (duh).
 */
#define ER_WARN(...)  ER_WARN_IMPL(RCSW_ER_PLUGIN_HANDLE(RCSW_ER_MODID, \
                                                         RCSW_ER_MODNAME), \
                                   __VA_ARGS__)


/* \cond INTERNAL */
#define ER_WARN_IMPL(handle, ...) {                                     \
    if (RCSW_ER_PLUGIN_LVL_CHECK(handle, WARN)) {        \
      ER_REPORT(WARN, handle, ## __VA_ARGS__)                           \
    }                                                                   \
  }
/* \endcond */
#endif /* RCSW_ERL >= RCSW_ERL_WARN */

/*******************************************************************************
 * Macros when ER is enabled for severity level >= INFO:
 *
 * - The configured plugin is used. Only [FATAL,ERROR,WARN,INFO] events are
 *   compiled in; others are discarded.
 *
 * - Debug printing macros enabled.
 ******************************************************************************/
#if (RCSW_ERL >= RCSW_ERL_INFO)

/**
 * \def ER_INFO(...)
 *
 * Report a INFOrmational message.
 */
#define ER_INFO(...)  ER_INFO_IMPL(RCSW_ER_PLUGIN_HANDLE(RCSW_ER_MODID, \
                                                         RCSW_ER_MODNAME), \
                                   __VA_ARGS__)

/* \cond INTERNAL */
#define ER_INFO_IMPL(handle, ...) {                                     \
    if (RCSW_ER_PLUGIN_LVL_CHECK(handle, INFO)) {        \
      ER_REPORT(INFO, handle, ## __VA_ARGS__)                           \
    }                                                                   \
  }
/* \endcond */
#endif /* RCSW_ERL >= RCSW_ERL_INFO */

/*******************************************************************************
 * Macros when ER is enabled for severity level >= DEBUG:
 *
 * - The configured plugin is used. Only [FATAL,ERROR,WARN,INFO,DEBUG] events
 *   are compiled in; others are discarded.
 *
 * - Debug printing macros enabled.
 ******************************************************************************/
#if (RCSW_ERL >= RCSW_ERL_DEBUG)

/**
 * \def ER_DEBUG(...)
 *
 * Report a DEBUGging message.
 */
#define ER_DEBUG(...)  ER_DEBUG_IMPL(RCSW_ER_PLUGIN_HANDLE(RCSW_ER_MODID, \
                                                         RCSW_ER_MODNAME), \
                                   __VA_ARGS__)

/* \cond INTERNAL */
#define ER_DEBUG_IMPL(handle, ...) {                                     \
    if (RCSW_ER_PLUGIN_LVL_CHECK(handle, DEBUG)) {        \
      ER_REPORT(DEBUG, handle, ## __VA_ARGS__)                           \
    }                                                                   \
  }
/* \endcond */
#endif /* RCSW_ERL >= RCSW_ERL_DEBUG */

/*******************************************************************************
 * Macros when ER is enabled for severity level >= TRACE:
 *
 * - The configured plugin is used. Only [FATAL,ERROR,WARN,INFO,DEBUG,TRACE]
 *   events are compiled in; others are discarded.
 *
 * - Debug printing macros enabled.
 ******************************************************************************/
#if (RCSW_ERL >= RCSW_ERL_TRACE)

/**
 * \def ER_TRACE(...)
 *
 * Report a TRACE message.
 */
#define ER_TRACE(...)  ER_TRACE_IMPL(RCSW_ER_PLUGIN_HANDLE(RCSW_ER_MODID, \
                                                         RCSW_ER_MODNAME), \
                                   __VA_ARGS__)

/* \cond INTERNAL */
#define ER_TRACE_IMPL(handle, ...) {                                     \
    if (RCSW_ER_PLUGIN_LVL_CHECK(handle, TRACE)) {        \
      ER_REPORT(TRACE, handle, ## __VA_ARGS__)                           \
    }                                                                   \
  }
/* \endcond */
#endif /* RCSW_ERL >= RCSW_ERL_TRACE */


/*******************************************************************************
 * General ER macros for when ER is != NONE.
 ******************************************************************************/
#if RCSW_ERL != RCSW_ERL_NONE

/**
 * \def ER_REPORT(lvl, msg, ...)
 *
 * Define a statement reporting the occurrence of an event with the specified
 * level \a lvl.
 *
 * This macro is only available if the event reporting level is > NONE.
 */
#define ER_REPORT(lvl, handle, msg, ...)        \
  {                                             \
    RCSW_ER_PLUGIN_REPORT(lvl,                  \
                          handle,               \
                          RCSW_ER_MODID,        \
                          RCSW_ER_MODNAME,      \
                          msg "\r\n",           \
                          ## __VA_ARGS__)       \
  }

#endif /* (RCSW_ERL != RCSW_ERL_NONE) */

/*******************************************************************************
 * General ER macros independent of level
 ******************************************************************************/
/**
 * \brief Platform/OS independent macro for printing to the terminal
 */
#define PRINTF(...) RCSW_ER_PLUGIN_PRINTF(__VA_ARGS__)

/**
 * \def ER_ASSERT(cond, msg, ...)
 *
 * Check a boolean condition \a cond in a function, halting the program if the
 * condition is not true. Like assert(), but allows for an additional custom
 * msg to be logged.
 */
/*
 * Don't define the macro to be nothing, as that can leave tons of "unused
 * variable" warnings in the code for variables which are only used in
 * asserts. The sizeof() trick here does *NOT* actually evaluate the
 * condition--only the size of whatever it returns. The variables are "used",
 * making the compiler happy, but ultimately removed by the optimizer.
 */
#define ER_ASSERT(cond, msg, ...)               \
  do {                                          \
    (void)sizeof((cond));                       \
    if (RCSW_UNLIKELY(!(cond))) {               \
      ER_FATAL( msg, ##__VA_ARGS__);            \
      assert(cond);                             \
    }                                           \
  } while (0);

/**
 * \def ER_CONDW(cond, msg, ...)
 *
 * Check a boolean condition \a cond in a function. If condition IS true,
 * emit a warning message.
 */
#define ER_CONDW(cond, msg, ...)                \
  {                                             \
    if (RCSW_LIKELY((cond))) {                  \
      ER_WARN(msg, ##__VA_ARGS__);              \
    }                                           \
  }

/**
 * \def ER_CONDI(cond, msg, ...)
 *
 * Check a boolean condition \a cond in a function. If condition IS true,
 * emit an informational message.
 */
#define ER_CONDI(cond, msg, ...)                \
  {                                             \
    if (RCSW_LIKELY((cond))) {                  \
      ER_INFO(msg, ##__VA_ARGS__);              \
    }                                           \
  }

/**
 * \def ER_CONDD(cond, msg, ...)
 *
 * Check a boolean condition \a cond in a function. If condition IS true,
 * emit a debug message.
 */
#define ER_CONDD(cond, msg, ...)                \
  {                                             \
    if (RCSW_LIKELY((cond))) {                  \
      ER_DEBUG(msg, ##__VA_ARGS__);             \
    }                                           \
  }



/**
 * \def ER_FATAL_SENTINEL(msg,...)
 *
 * Mark a place in the code as being universally bad, like really really
 * bad. Fatally bad. If execution ever reaches this spot stop the program after
 * reporting the specified message.
 */
#define ER_FATAL_SENTINEL(msg, ...)             \
  {                                             \
    ER_FATAL(msg, ##__VA_ARGS__);               \
    abort();                                    \
  }


/**
 * \def ER_CHECK(cond, msg, ...)
 *
 * Check a boolean condition \a cond in a function. If condition is not true, go
 * to the error/bailout section for function (you must have a label called \c
 * error in your function) after reporting the event.
 */
#define ER_CHECK(cond, msg, ...)                \
  {                                             \
    if (RCSW_UNLIKELY(!(cond))) {               \
      ER_ERR(msg, ##__VA_ARGS__);               \
      goto error;                               \
    }                                           \
  }

/**
 * \def ER_SENTINEL(msg,...)
 *
 * Mark a place in the code as being universally bad. If execution ever reaches
 * this spot, report the event and error out (you must have a label called \c
 * error in your function).
 */
#define ER_SENTINEL(msg, ...)                   \
  {                                             \
    ER_ERR(msg, ##__VA_ARGS__);                 \
    goto error;                                 \
  }

/*******************************************************************************
 * Macro Cleanup
 *
 * Depending on compile-time level, one or more of these macros may be
 * undefined, so make sure everything is defined so things build cleanly.
 ******************************************************************************/
#ifndef ER_FATAL
#define ER_FATAL(...)
#endif

#ifndef ER_ERR
#define ER_ERR(...)
#endif

#ifndef ER_WARN
#define ER_WARN(...)
#endif

#ifndef ER_INFO
#define ER_INFO(...)
#endif

#ifndef ER_DEBUG
#define ER_DEBUG(...)
#endif

#ifndef ER_TRACE
#define ER_TRACE(...)
#endif

#ifndef RCSW_ER_MODULE_INIT
#define RCSW_ER_MODULE_INIT(...)
#endif

#ifndef RCSW_ER_INIT
#define RCSW_ER_INIT(...)
#endif

#ifndef RCSW_ER_DEINIT
#define RCSW_ER_DEINIT(...)
#endif

#ifndef RCSW_ER_INSMOD
#define RCSW_ER_INSMOD(...)
#endif

#ifndef ER_REPORT
#define ER_REPORT(...)
#endif

#ifndef DPRINTF
#define DPRINTF(...)
#endif

#ifndef DPRINT_TOK
#define DPRINT_TOK(...)
#endif

#ifndef DPRINT_TOKD
#define DPRINT_TOKD(...)
#endif

#ifndef DPRINT_TOKX
#define DPRINT_TOKX(...)
#endif

#ifndef DPRINT_TOKF
#define DPRINT_TOKF(...)
#endif
