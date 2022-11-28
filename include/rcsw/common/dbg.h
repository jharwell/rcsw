/**
 * \file dbg.h
 * \ingroup common
 * \brief A simple C debugging/logging framework.
 *
 * Comprises debug printing on a module basis, with the capability to set the
 * level for each module independently. All debug printing can be compiled out
 * for maximum speed by passing NDEBUG.
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef INCLUDE_RCSW_COMMON_DBG_H_
#define INCLUDE_RCSW_COMMON_DBG_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "rcsw/common/common.h"
#include "rcsw/ds/llist.h"

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
/** \cond INTERNAL */

/** Used when you want to turn debugging OFF for all modules */
#define DBG_OFF 6

/**
 * Used to print things regarding errors, as in something bad happened in your
 * program.
 */
#define DBG_E 5

/**
 * Used to print warning things, as in something happened athat is somewhat
 * alarming/sub-optimal, but not in of itself catastrophic/fatal.
 */
#define DBG_W 4

/**
 * Normal/nominative printing. Use this for things that you want your program to
 * print when everything is going smoothly. According to UNIX philosophy, if
 * your program has nothing interesting to say, it should say nothing if this is
 * the debugging level.
 */
#define DBG_N 3

/**
 * Used to print diagnostic information, as in additional info that
 * a developer can use to help debug a problem.
 */
#define DBG_D 2

/**
 * Used to print verbose information, for use on tough problems when you want
 * the use to get as much information from your program as possible.
 */
#define DBG_V 1

/** \endcond */

/**
 * \brief Debug color codes (for producing colored terminal output)
 */
#define DBG_HEADC "\033[36m" /* blue */
#define DBG_OKC "\033[32m"   /* green */
#define DBG_WARNC "\033[33m" /* yellow */
#define DBG_FAILC "\033[31m" /* red */
#define DBG_ENDC "\033[0m"   /* reset to default terminal text color */

/*******************************************************************************
 * Platform Independent Printing
 ******************************************************************************/
/**
 * \brief Platform/OS independent macro for printing to the terminal
 */
#if defined(__linux__)
#define PRINTF(...) printf(__VA_ARGS__);
#endif

/**
 * \brief Print floating point numbers on platforms where using %f directly in
 * printf()/printk() doesn't work.
 */
#define PRINTF_FLOAT(...)                               \
  {                                                     \
    char fpstr[128];                                    \
    snprintf(fpstr, sizeof(fpstr), __VA_ARGS__);        \
    PRINTF("%s", fpstr);                                \
  }

/**
 * \brief Print 64 bit integers on 32 bit platforms where printing 64 bits
 * directly in printf()/printk() doesn't work.
 */
#define PRINTF64(...)                                   \
  {                                                     \
    char str64[128];                                    \
    snprintf(str64, sizeof(str64), __VA_ARGS__);        \
    PRINTF("%s", str64);                                \
  }

/*******************************************************************************
 * Debug Macros
 ******************************************************************************/
/*
 * If NDEBUG is defined, then all debugging macros compile out
 */
#if defined(NDEBUG)

#define DBGE(...)
#define DBGW(...)
#define DBGN(...)
#define DBGD(...)
#define DBGV(...)

#define DBGTE(...)
#define DBGTW(...)
#define DBGTN(...)
#define DBGTD(...)
#define DBGTV(...)

#define DEBUG(level, args, ...)
#define DEBUGT(level, args, ...)

#define DPRINTF(args...)
#define DPRINTF_FLOAT(args...)

#else /* debugging is enabled */

/**
 * \brief General debug macros that will display whenever debugging is enabled,
 * regardless of the value of DBG_LVL.
 */
#define DPRINTF(...) PRINTF(__VA_ARGS__)
#define DPRINTF_FLOAT(...) PRINTF_FLOAT(__VA_ARGS__)

/**
 * \brief Debug level statements that have the level encoded into the macro
 * itself so you don't have to type it in yourself.
 */
#define DBGE(...) DEBUG(DBG_E, __VA_ARGS__)
#define DBGW(...) DEBUG(DBG_W, __VA_ARGS__)
#define DBGN(...) DEBUG(DBG_N, __VA_ARGS__)
#define DBGD(...) DEBUG(DBG_D, __VA_ARGS__)
#define DBGV(...) DEBUG(DBG_V, __VA_ARGS__)

#define DBGTE(...) DEBUGT(DBG_E, __VA_ARGS__)
#define DBGTW(...) DEBUGT(DBG_W, __VA_ARGS__)
#define DBGTN(...) DEBUGT(DBG_N, __VA_ARGS__)
#define DBGTD(...) DEBUGT(DBG_D, __VA_ARGS__)
#define DBGTV(...) DEBUGT(DBG_V, __VA_ARGS__)

/**
 * \brief Regular debug statement. It will only be printed is debug printing for
 * the module is enabled AND the debug level for the statement is high enough.
 *
 * It will prepend the string of the module name, followed by a colon and a
 * space, before the actual debug statement.
 */
#define DEBUG(lvl, ...)                                                 \
  {                                                                     \
    struct dbg_module mod_in;                                           \
    mod_in.id_ = MODULE_ID;                                             \
    mod_in.name_[0] = '\0';                                             \
    if (dbg_g.modules_) {                                               \
      struct dbg_module* mod_out  =                                     \
                                                                  (struct dbg_module*)llist_data_query( \
                                                                      dbg_g.modules_, &mod_in); \
      if ((NULL != mod_out) && (mod_out->lvl_ <= lvl)) {                \
        char str[1000];                                                 \
        snprintf(str, sizeof(str), "%s: ", mod_out->name_);             \
        snprintf(str+strlen(str), sizeof(str), __VA_ARGS__);            \
        PRINTF("%s", str);                                              \
        fflush(NULL);                                                   \
      }                                                                 \
    }                                                                   \
  }

/**
 * \brief Timestamped debug statement.
 *
 * The timestamp has a maximum resolution of 10000 seconds; after that it rolls
 * over. The decimal portion of the timestamp reports microseconds, though
 * depending on OS, processor speed, etc. you may only be able to reliably trust
 * measurement down to the milliseconds.
 */
#define DEBUGT(lvl, ...)                                                \
  {                                                                     \
    struct dbg_module mod_in;                                           \
    mod_in.id_ = MODULE_ID;                                             \
    mod_in.name_[0] = '\0';                                             \
    if (dbg_g.modules_) {                                               \
      struct dbg_module* mod  = (struct dbg_module*)llist_data_query(   \
          dbg_g.modules_, &mod_in);                                     \
      if ((NULL != mod) && (mod->lvl_ <= lvl)) {                        \
        struct timespec _curr_time;                                     \
        char str[1000];                                                 \
        clock_gettime(CLOCK_REALTIME, &_curr_time);                     \
        snprintf(str, sizeof(str), "[%04zu.%06zu] ",                    \
                 _curr_time.tv_sec % 10000, _curr_time.                 \
                 tv_nsec / 1000);                                       \
        snprintf(str+strlen(str), sizeof(str), "%s: ", mod->name_);     \
        snprintf(str+strlen(str), sizeof(str), __VA_ARGS__);            \
        PRINTF("%s", str);                                              \
        fflush(NULL);                                                   \
      }                                                                 \
    }                                                                   \
  }

#endif /* NDEBUG */

/**
 * \brief Like \ref RCSW_CHECK(), but has an additional message that is printed on failure
 * if debugging is enabled for the module it resides in.
 */
#define SOFT_ASSERT(cond, msg, ...)                             \
  if (!(cond)) {                                                \
    DBGE(__FILE__ ":%s:%d " msg "\n", __func__, __LINE__,       \
         ##__VA_ARGS__);                                        \
    goto error;                                                 \
  }

/**
 * \brief Mark a place in the code as being universally bad. If execution ever
 * reaches this spot, print a message and error out.
 */
#define SENTINEL(msg, ...)                      \
  {                                             \
    DBGE(msg "\n", ##__VA_ARGS__);              \
    goto error;                                 \
  }

/*******************************************************************************
 * String Macros
 ******************************************************************************/
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
#define PRINT_TOKF(tok) DPRINTF_FLOAT(STR(tok) ": %.8f\n", (float)(tok));

/*******************************************************************************
 * Module codes
 ******************************************************************************/
/**
 * \brief The debug module codes used by rcsw.
 *
 * When defining your own module codes, you should always start them with
 * M_EXTERNAL, so as to not conflict with the internal codes in rcsw.
 */
#define RCSW_DBG_MODULES                        \
  M_DBG,                                        \
    M_DS_BSTREE,                                \
    M_DS_DARRAY,                                \
    M_DS_LLIST,                                 \
    M_DS_HASHMAP,                               \
    M_DS_RBUFFER,                               \
    M_MT_QUEUE,                                 \
    M_DS_MPOOL,                                 \
    M_UTILS,                                    \
    M_PULSE,                                    \
    M_STDIO,                                    \
    M_DS_CORE,                                  \
    M_DS_BIN_HEAP,                              \
    M_DS_CSMATRIX,                              \
    M_COMMON,                                   \
    M_DS_FIFO,                                  \
    M_DS_RAWFIFO,                               \
    M_ALGORITHM,                                \
    M_DS_RBTREE,                                \
    M_TESTING,                                  \
    M_DS_INT_TREE,                              \
    M_DS_OSTREE,                                \
    M_DS_STATIC_ADJ_MATRIX,                     \
    M_DS_STATIC_MATRIX,                         \
    M_DS_DYNAMIC_MATRIX,                        \
    M_MULTITHREAD,                              \
    M_MULTIPROCESS,                             \
    M_EXTERNAL

enum dbg_module_codes {RCSW_XGEN_ENUMS(RCSW_DBG_MODULES)};

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/
/**
 * \brief Representation of a module for debugging.
 *
 * A module is defined on a per file basis (multiple modules in the same file
 * are disallowed). This is so that I can make use of the preprocessor to
 * generate the necessary code for debugging statements easily.
 */
struct dbg_module {
  int64_t id_; /* id_ must be the first field for comparisons to work */
  uint8_t lvl_; /* The current debugging level */
  char name_[20]; /* The name of the module */
};

/**
 * \brief The debugging framework
 *
 * The list of modules currently enabled is maintained by a linked list, and the
 * framework also contains a default level that can be set so that all future
 * modules will be installed with that level by default.
 */
struct dbg_frmwk {
  struct llist *modules_;
  uint8_t default_lvl_;
  bool_t initialized_;
};


/*******************************************************************************
 * Global Variables
 ******************************************************************************/
BEGIN_C_DECLS
extern struct dbg_frmwk dbg_g;

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
/**
 * \brief Initialize Debugging Framework
 *
 * This function is idempotent.
 *
 * \return \ref status_t
 */
status_t dbg_init(void);

/**
 * \brief Shutdown the debugging framework, deallocating memory.
 *
 * The framework can be re-initialized later without error.
 */
void dbg_shutdown(void);

/**
 * \brief Add a module to the active list of debug printing modules
 *
 * If the module already exists, ERROR is returned.
 *
 * \param id A UUID for the module to be installed
 * \param name The name of the debugging module. This will be prepended to all
 * debug printing messages. Names do not necessarily have to be unique within
 * the application, though it's a good idea to make them that way.
 *
 * \return \ref status_t
 */
status_t dbg_insmod(int64_t id, const char *name);

/**
 * \brief Remove a module from the active list by id. If the module is not in
 * the list success, not failure, is returned.
 *
 * \param id The UUID of the module to remove
 * \return \ref status_t
 */
status_t dbg_rmmod(int64_t id);

/**
 * \brief Remove a module from the active list by name. If the module is not in
 * the list success, not failure, is returned.
 *
 * \param name The name of the module to remove
 * \return \ref status_t
 */
status_t dbg_rmmod2(const char *name);

/**
 * \brief Set the debugging level for a module
 *
 * \return \ref status_t
 */
status_t dbg_mod_lvl_set(int64_t id, uint8_t lvl);

/**
 * \brief Set the default debugging level for the debugging framework.
 *
 * All modules installed after calling this function will have the specified
 * level set by default.
 *
 * \param lvl The new default level (one of DBG_OFF, DBG_E, DBG_W, etc.)
 *
 */
static inline void dbg_default_lvl_set(uint8_t lvl) {
  dbg_g.default_lvl_ = lvl;
}

/**
 * \brief Get the ID of a module from its name
 *
 * \param name The name of the module to retrieve the UUID for
 *
 * \return The ID, or -1 if an error occurred.
 */
int64_t dbg_mod_id_get(const char* name) RCSW_PURE;

END_C_DECLS

#endif /* INCLUDE_RCSW_COMMON_DBG_H_ */
