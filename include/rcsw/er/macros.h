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
 * Macros
 ******************************************************************************/

#if (RCSW_ER == RCSW_ER_NONE)

#define RCSW_ER_MODULE_INIT(...)

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
  } while (0)

#define ER_FATAL(...)

#define ER_ERR(...)
#define ER_WARN(...)
#define ER_INFO(...)
#define ER_DEBUG(...)
#define ER_TRACE(...)
#define ER_CONDW(...)
#define ER_CONDI(...)
#define ER_CONDD(...)
#define ER_REPORT(...)
#define DPRINTF(...)
#define DPRINTFF(...)

#elif (RCSW_ER == RCSW_ER_FATAL)

#define ER_ERR(...)
#define ER_WARN(...)
#define ER_INFO(...)
#define ER_DEBUG(...)
#define ER_TRACE(...)
#define ER_REPORT(...)
#define ER_CONDI(...)
#define ER_CONDW(...)
#define ER_CONDD(...)
#define RCSW_ER_MODULE_INIT(...)

#define ER_FATAL(msg, ...)                                              \
    {                                                                   \
      DPRINTF(RCSW_ER_MODNAME " [FATAL]: " msg, ## __VA_ARGS__);        \
    }                                                                   \


/**
 * \def ER_ASSERT(cond, msg, ...)
 *
 * Check a boolean condition \a cond in a function, halting the program if the
 * condition is not true. Like assert(), but allows for an additional custom
 * msg to be logged.
 *
 * You can use this macro in non-class contexts.
 */
#define ER_ASSERT(cond, msg, ...) {             \
    if (RCSW_UNLIKELY(!(cond))) {               \
      ER_FATAL(msg, ##__VA_ARGS__);             \
    }                                           \
  }

#elif (RCSW_ER == RCSW_ER_ALL)

/**
 * \def RCSW_ER_MODULE_INIT()
 *
 * Initialize a module in the currently selected ER plugin.
 *
 * Installation is idempotent. Requires that \ref RCSW_ER_MODID and \ref
 * RCSW_ER_MODNAME if they are used by the plugin; otherwise, they can be
 * undefined.
 *
 * This macro is only available if event reporting is fully enabled.
 */
#if !defined(RCSW_ER_MODNAME)
#define RCSW_ER_MODNAME __FILE_NAME__
#endif

#define RCSW_ER_MODULE_INIT(...)                        \
  RCSW_ER_PLUGIN_INSMOD(RCSW_ER_MODID, RCSW_ER_MODNAME)

/**
 * \def ER_REPORT(lvl, msg, ...)
 *
 * Define a statement reporting the occurrence of an event with the specified
 * level \a lvl.
 *
 * This macro is only available if event reporting is fully enabled.
 */

#define ER_REPORT(lvl, handle, msg, ...)        \
  {                                             \
    RCSW_ER_PLUGIN_REPORT(lvl,                  \
                          handle,               \
                          RCSW_ER_MODID,        \
                          RCSW_ER_MODNAME,      \
                          msg "\n",             \
                          ## __VA_ARGS__)       \
  }

#define ER_INFO_IMPL(handle, ...) {                                     \
    if (RCSW_ER_PLUGIN_LVL_CHECK(handle, RCSW_ER_PLUGIN_INFO)) {        \
      ER_REPORT(INFO, handle, __VA_ARGS__)                              \
    }                                                                   \
  }
#define ER_ERR_IMPL(handle, ...) {                                      \
    if (RCSW_ER_PLUGIN_LVL_CHECK(handle, RCSW_ER_PLUGIN_ERROR)) {       \
      ER_REPORT(ERROR, handle, __VA_ARGS__)                             \
    }                                                                   \
  }

#define ER_WARN_IMPL(handle, ...) {                                     \
    if (RCSW_ER_PLUGIN_LVL_CHECK(handle, RCSW_ER_PLUGIN_WARN)) {        \
      ER_REPORT(WARN, handle, ## __VA_ARGS__)                           \
    }                                                                   \
  }
#define ER_DEBUG_IMPL(handle, ...) {                                    \
    if (RCSW_ER_PLUGIN_LVL_CHECK(handle, RCSW_ER_PLUGIN_DEBUG)) {       \
      ER_REPORT(DEBUG, handle, __VA_ARGS__)                             \
    }                                                                   \
  }
#define ER_TRACE_IMPL(handle, ...) {                                    \
    if (RCSW_ER_PLUGIN_LVL_CHECK(handle, RCSW_ER_PLUGIN_TRACE)) {       \
      ER_REPORT(TRACE, handle, __VA_ARGS__)                             \
    }                                                                   \
  }

/**
 * \def ER_ERR(...)
 *
 * Report a non-FATAL ERROR message.
 */
#define ER_ERR(...)  ER_ERR_IMPL(RCSW_ER_PLUGIN_HANDLE(RCSW_ER_MODID,   \
                                                       RCSW_ER_MODNAME), \
                                 __VA_ARGS__)

/**
 * \def ER_WARN(...)
 *
 * Report a WARNING message (duh).
 */
#define ER_WARN(...)  ER_WARN_IMPL(RCSW_ER_PLUGIN_HANDLE(RCSW_ER_MODID, \
                                                         RCSW_ER_MODNAME), \
                                   __VA_ARGS__)

/**
 * \def ER_INFO(...)
 *
 * Report an INFOrmational message.
 */
#define ER_INFO(...)  ER_INFO_IMPL(RCSW_ER_PLUGIN_HANDLE(RCSW_ER_MODID, \
                                                         RCSW_ER_MODNAME), \
                                   __VA_ARGS__)

/**
 * \def ER_DEBUG(...)
 *
 * Report a DEBUG message.
 */
#define ER_DEBUG(...)  ER_DEBUG_IMPL(RCSW_ER_PLUGIN_HANDLE(RCSW_ER_MODID, \
                                                           RCSW_ER_MODNAME), \
                                     __VA_ARGS__)

/**
 * \def ER_TRACE(...)
 *
 * Report a TRACE message.
 */
#define ER_TRACE(...)  ER_TRACE_IMPL(RCSW_ER_PLUGIN_HANDLE(RCSW_ER_MODID, \
                                                           RCSW_ER_MODNAME), \
                                     __VA_ARGS__)

/**
 * \def ER_ASSERT(cond, msg, ...)
 *
 * Check a boolean condition \a cond in a function, halting the program if the
 * condition is not true. Like assert(), but allows for an additional custom
 * msg to be logged.
 *
 * You cannot use this macro in non-class contexts, and all classes using it
 * must derive from \ref client.
 */
#define ER_ASSERT(cond, msg, ...)               \
  if (RCSW_UNLIKELY(!(cond))) {                 \
    ER_FATAL( msg, ##__VA_ARGS__);              \
    assert(cond);                               \
  }

/**
 * \def ER_CHECKW(cond, msg, ...)
 *
 * Check a boolean condition \a cond in a function. If condition IS true,
 * emit a warning message.
 *
 * You cannot use this macro in non-class contexts, and all classes using it
 * must derive from \ref client. This macro is only available if event reporting
 * is fully enabled.
 */
#define ER_CONDW(cond, msg, ...)                \
  {                                             \
    if (RCSW_LIKELY((cond))) {                  \
      ER_WARN(msg, ##__VA_ARGS__);              \
    }                                           \
  }

/**
 * \def ER_CHECKI(cond, msg, ...)
 *
 * Check a boolean condition \a cond in a function. If condition IS true,
 * emit an informational message.
 *
 * You cannot use this macro in non-class contexts, and all classes using it
 * must derive from \ref client. This macro is only available if event reporting
 * is fully enabled.
 */
#define ER_CONDI(cond, msg, ...)                \
  {                                             \
    if (RCSW_LIKELY((cond))) {                  \
      ER_INFO(msg, ##__VA_ARGS__);              \
    }                                           \
  }

/**
 * \def ER_CHECKD(cond, msg, ...)
 *
 * Check a boolean condition \a cond in a function. If condition IS true,
 * emit a debug message.
 *
 * You cannot use this macro in non-class contexts, and all classes using it
 * must derive from \ref client. This macro is only available if event reporting
 * is fully enabled.
 */
#define ER_CONDD(cond, msg, ...)                \
  {                                             \
    if (RCSW_LIKELY((cond))) {                  \
      ER_DEBUG(msg, ##__VA_ARGS__);             \
    }                                           \
  }


#endif /* (RCSW_ER == RCSW_ER_ALL) */

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

/**
 * \brief Platform/OS independent macro for printing to the terminal
 */
#define PRINTF(...) RCSW_ER_PRINTF(__VA_ARGS__)

#if RCSW_ER >= RCSW_ER_FATAL

/**
 * \brief General debug macros that will display whenever logging is enabled,
 * any other settings.
 */
#define DPRINTF(...) PRINTF(__VA_ARGS__)

/**
 * \brief Print a token AND it's value. Useful for debugging. Comes in the
 * following flavors:
 *
 * TOK  - decimal/hexadecimal
 * TOKD - decimal
 * TOKX - hexadecimal
 * TOKF - float
 */
#define PRINT_TOK(tok) DPRINTF(STR(tok) ": %d 0x%x\n", (int)(tok), (int)(tok));
#define PRINT_TOKD(tok) DPRINTF(STR(tok) ": %d\n", (int)(tok));
#define PRINT_TOKX(tok) DPRINTF(STR(tok) ": 0x%x\n", (int)(tok));
#define PRINT_TOKF(tok) DPRINTF(STR(tok) ": %.8f\n", (float)(tok));

#endif /* RCSW_ER >= RCSW_ER_FATAL */
