/**
 * \file
 *
 * \copyright 2023 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * \ingroup er
 *
 * \brief Event reporting macros (ER_DEBUG(), ER_WARN(), ...).
 *
 * Each \c ER_<LEVEL>() macro expands to nothing when \ref RCSW_ERL is below
 * that level, so disabled events cost nothing. See
 * \rcswdoc{concepts/event-reporting/levels}.
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <assert.h>
#include <errno.h>

#include "rcsw/core/compilers.h"
#include "rcsw/er/er.h"

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
 * The arguments are passed to the plugin's init function: none for the simple
 * and LOG4CL plugins, the configuration file path for zlog. All usage of any
 * ER machinery is undefined until this call.
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
 * \brief Print through \ref PRINTF() whenever \ref RCSW_ERL is at least FATAL,
 * regardless of module levels.
 */
#define DPRINTF(...) PRINTF(__VA_ARGS__)

/**
 * \brief Print a token and its value in decimal/hexadecimal.
 */
#define DPRINT_TOK(tok) \
  DPRINTF(RCSW_XSTR(tok) ": %d/0x%x\n", (int)(tok), (int)(tok));

/**
 * \brief Print a token and its value in decimal.
 */
#define DPRINT_TOKD(tok) DPRINTF(RCSW_XSTR(tok) ": %d\n", (int)(tok));

/**
 * \brief Print a token and its value in hexadecimal.
 */
#define DPRINT_TOKX(tok) DPRINTF(RCSW_XSTR(tok) ": 0x%x\n", (int)(tok));

/**
 * \brief Print a token and its value in floating point.
 */
#define DPRINT_TOKF(tok) DPRINTF(RCSW_XSTR(tok) ": %.8f\n", (float)(tok));

#endif /* RCSW_ERL >= RCSW_ERL_FATAL */

/*******************************************************************************
 * Module identity: defaulted at every ER level, since ER_FATAL needs it even
 * when RCSW_ERL == RCSW_ERL_FATAL.
 ******************************************************************************/
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
 * \brief The ID of an ER module. Used by some logging plugins (e.g., LOG4CL) to
 * identify a module.
 *
 * If not specified, defined as 0xFFFFFFFF. That value is above every ID RCSW
 * uses internally, so it cannot collide with them; all translation units which
 * do not define their own ID share it (and therefore share one LOG4CL module).
 */
#if !defined(RCSW_ER_MODID)
#define RCSW_ER_MODID (0xFFFFFFFF)
#endif

/**
 * \def ER_FATAL(msg, ...)
 *
 * Report a FATAL message. With \ref RCSW_ERL above FATAL it goes through the
 * ER plugin, like the other levels. With \ref RCSW_ERL equal to FATAL no
 * plugin is used and the message is printed with \ref DPRINTF().
 */
#if (RCSW_ERL == RCSW_ERL_FATAL)

#define ER_FATAL(msg, ...)                                    \
  {                                                           \
    DPRINTF(RCSW_ER_MODNAME " [FATAL]: " msg, ##__VA_ARGS__); \
  }

/** \cond INTERNAL */
#define RCSW_ER_MODULE_INIT(...)
/** \endcond */

#elif (RCSW_ERL > RCSW_ERL_FATAL)

#define ER_FATAL(...)                                                  \
  ER_FATAL_IMPL(RCSW_ER_PLUGIN_HANDLE(RCSW_ER_MODID, RCSW_ER_MODNAME), \
                __VA_ARGS__)

/* \cond INTERNAL */
#define ER_FATAL_IMPL(handle, ...)                 \
  {                                                \
    if (RCSW_ER_PLUGIN_LVL_CHECK(handle, FATAL)) { \
      ER_REPORT(FATAL, handle, __VA_ARGS__)        \
    }                                              \
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
#define ER_ERR(...) \
  ER_ERR_IMPL(RCSW_ER_PLUGIN_HANDLE(RCSW_ER_MODID, RCSW_ER_MODNAME), __VA_ARGS__)

/* \cond INTERNAL */
#define ER_ERR_IMPL(handle, ...)                   \
  {                                                \
    if (RCSW_ER_PLUGIN_LVL_CHECK(handle, ERROR)) { \
      ER_REPORT(ERROR, handle, __VA_ARGS__)        \
    }                                              \
  }
/* \endcond */

/**
 * \def RCSW_ER_MODULE_INIT()
 *
 * Initialize a module in the currently selected ER plugin using the \ref
 * RCSW_ER_MODID and \ref RCSW_ER_MODNAME currently in scope.
 *
 * Initialization is idempotent if the selected plugin supports it.
 * Registration is best-effort bookkeeping (e.g., the plugin may not be
 * initialized yet), so \c errno is preserved across the call.
 */
/* INSMOD may expand to nothing (e.g., simple plugin), so no (void) cast */
#define RCSW_ER_MODULE_INIT(...)                           \
  do {                                                     \
    int rcsw_er_saved_errno_ = errno;                      \
    RCSW_ER_PLUGIN_INSMOD(RCSW_ER_MODID, RCSW_ER_MODNAME); \
    errno = rcsw_er_saved_errno_;                          \
  } while (0)

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
 * Report a WARNING message.
 */
#define ER_WARN(...) \
  ER_WARN_IMPL(RCSW_ER_PLUGIN_HANDLE(RCSW_ER_MODID, RCSW_ER_MODNAME), __VA_ARGS__)

/* \cond INTERNAL */
#define ER_WARN_IMPL(handle, ...)                 \
  {                                               \
    if (RCSW_ER_PLUGIN_LVL_CHECK(handle, WARN)) { \
      ER_REPORT(WARN, handle, ##__VA_ARGS__)      \
    }                                             \
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
 * Report an informational message.
 */
#define ER_INFO(...) \
  ER_INFO_IMPL(RCSW_ER_PLUGIN_HANDLE(RCSW_ER_MODID, RCSW_ER_MODNAME), __VA_ARGS__)

/* \cond INTERNAL */
#define ER_INFO_IMPL(handle, ...)                 \
  {                                               \
    if (RCSW_ER_PLUGIN_LVL_CHECK(handle, INFO)) { \
      ER_REPORT(INFO, handle, ##__VA_ARGS__)      \
    }                                             \
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
 * Report a debug message.
 */
#define ER_DEBUG(...)                                                  \
  ER_DEBUG_IMPL(RCSW_ER_PLUGIN_HANDLE(RCSW_ER_MODID, RCSW_ER_MODNAME), \
                __VA_ARGS__)

/* \cond INTERNAL */
#define ER_DEBUG_IMPL(handle, ...)                 \
  {                                                \
    if (RCSW_ER_PLUGIN_LVL_CHECK(handle, DEBUG)) { \
      ER_REPORT(DEBUG, handle, ##__VA_ARGS__)      \
    }                                              \
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
#define ER_TRACE(...)                                                  \
  ER_TRACE_IMPL(RCSW_ER_PLUGIN_HANDLE(RCSW_ER_MODID, RCSW_ER_MODNAME), \
                __VA_ARGS__)

/* \cond INTERNAL */
#define ER_TRACE_IMPL(handle, ...)                 \
  {                                                \
    if (RCSW_ER_PLUGIN_LVL_CHECK(handle, TRACE)) { \
      ER_REPORT(TRACE, handle, ##__VA_ARGS__)      \
    }                                              \
  }
/* \endcond */
#endif /* RCSW_ERL >= RCSW_ERL_TRACE */

/*******************************************************************************
 * General ER macros for when ER is != NONE.
 ******************************************************************************/
#if RCSW_ERL != RCSW_ERL_NONE

/**
 * \def ER_REPORT(lvl, handle, msg, ...)
 *
 * Define a statement reporting the occurrence of an event with the specified
 * level \a lvl through the plugin \a handle. \c "\\r\\n" is appended to
 * \a msg, so messages should not end with a newline.
 *
 * This macro is only available if the event reporting level is > NONE.
 */
#define ER_REPORT(lvl, handle, msg, ...)  \
  {RCSW_ER_PLUGIN_REPORT(lvl,             \
                         handle,          \
                         RCSW_ER_MODID,   \
                         RCSW_ER_MODNAME, \
                         msg "\r\n",      \
                         ##__VA_ARGS__)}

#endif /* (RCSW_ERL != RCSW_ERL_NONE) */

/*******************************************************************************
 * General ER macros independent of level
 ******************************************************************************/
/**
 * \brief Print to the terminal, through the configured ER plugin's print
 * function.
 */
#define PRINTF(...) RCSW_ER_PLUGIN_PRINTF(__VA_ARGS__)

/**
 * \def ER_ASSERT(cond, msg, ...)
 *
 * Check a boolean condition \a cond in a function. If it is false, report
 * \a msg as a FATAL event and then fail an \c assert(), which halts the
 * program unless \c NDEBUG is defined. \a cond is evaluated in every build.
 */
/*
 * The (void)sizeof() keeps variables used only in the condition "used" even
 * if the rest of the expansion is optimized away.
 */
#define ER_ASSERT(cond, msg, ...)   \
  do {                              \
    (void)sizeof((cond));           \
    if (RCSW_UNLIKELY(!(cond))) {   \
      ER_FATAL(msg, ##__VA_ARGS__); \
      assert(cond);                 \
    }                               \
  } while (0)

/**
 * \def ER_CONDW(cond, msg, ...)
 *
 * Check a boolean condition \a cond in a function. If condition IS true,
 * emit a warning message.
 */
#define ER_CONDW(cond, msg, ...)   \
  {                                \
    if (RCSW_LIKELY((cond))) {     \
      ER_WARN(msg, ##__VA_ARGS__); \
    }                              \
  }

/**
 * \def ER_CONDI(cond, msg, ...)
 *
 * Check a boolean condition \a cond in a function. If condition IS true,
 * emit an informational message.
 */
#define ER_CONDI(cond, msg, ...)   \
  {                                \
    if (RCSW_LIKELY((cond))) {     \
      ER_INFO(msg, ##__VA_ARGS__); \
    }                              \
  }

/**
 * \def ER_CONDD(cond, msg, ...)
 *
 * Check a boolean condition \a cond in a function. If condition IS true,
 * emit a debug message.
 */
#define ER_CONDD(cond, msg, ...)    \
  {                                 \
    if (RCSW_LIKELY((cond))) {      \
      ER_DEBUG(msg, ##__VA_ARGS__); \
    }                               \
  }

/**
 * \def ER_FATAL_SENTINEL(msg,...)
 *
 * Mark a place in the code that must never be reached. If execution gets
 * there, report \a msg as a FATAL event and call \c abort().
 */
#define ER_FATAL_SENTINEL(msg, ...) \
  {                                 \
    ER_FATAL(msg, ##__VA_ARGS__);   \
    abort();                        \
  }

/**
 * \def ER_CHECK(cond, msg, ...)
 *
 * Check a boolean condition \a cond in a function. If it is false, report
 * \a msg as an ERROR event and go to the function's \c error label, which
 * must exist.
 */
#define ER_CHECK(cond, msg, ...)  \
  {                               \
    if (RCSW_UNLIKELY(!(cond))) { \
      ER_ERR(msg, ##__VA_ARGS__); \
      goto error;                 \
    }                             \
  }

/**
 * \def ER_SENTINEL(msg,...)
 *
 * Mark a place in the code that should not be reached. If execution gets
 * there, report \a msg as an ERROR event and go to the function's \c error
 * label, which must exist.
 */
#define ER_SENTINEL(msg, ...)   \
  {                             \
    ER_ERR(msg, ##__VA_ARGS__); \
    goto error;                 \
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

/** \cond INTERNAL */
#ifndef RCSW_ER_MODULE_INIT
#define RCSW_ER_MODULE_INIT(...)
#endif
/** \endcond */

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
