/**
 * \file dbg.h
 * \ingroup er
 * \brief A simple C debugging/logging frameworkwhich only uses RCSW internals.
 *
 * Comprises debug printing on a module basis, with the capability to set the
 * level for each module independently (if you really want to). Mainly intended
 * for environments with no stdlib, such as bootstraps.
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "rcsw/rcsw.h"
#include "rcsw/ds/llist.h"

/*******************************************************************************
 * RCSW ER Plugin Definitions
 ******************************************************************************/
#define RCSW_ER_PLUGIN_OFF   6
#define RCSW_ER_PLUGIN_ERROR 5
#define RCSW_ER_PLUGIN_WARN  4
#define RCSW_ER_PLUGIN_INFO  3
#define RCSW_ER_PLUGIN_DEBUG 2
#define RCSW_ER_PLUGIN_TRACE 1

#define RCSW_ER_PLUGIN_PRINTF printf

#define RCSW_ER_PLUGIN_REPORT(lvl, logger,  id, name, msg, ...) \
  {                                                             \
    RCSW_ER_PLUGIN_PRINTF(name " [" RCSW_XSTR(lvl) "] "  msg,   \
                          ## __VA_ARGS__);                      \
  }

#define RCSW_ER_PLUGIN_INSMOD(id, name) log4cl_insmod((id), (name))

#define RCSW_ER_PLUGIN_HANDLE(id, name) log4cl_mod_query(id)

#define RCSW_ER_PLUGIN_LVL_CHECK(handle, lvl) log4cl_mod_enabled(handle, lvl)


/*******************************************************************************
 * Module codes
 ******************************************************************************/
/**
 * \brief The debug module codes used by rcsw.
 *
 * When defining your own module codes, you should always start them with
 * M_EXTERNAL, so as to not conflict with the internal codes in rcsw.
 */
#define RCSW_LOG4CL_MODULES                        \
  M_LOG4CL,                                        \
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

enum log4cl_module_codes {RCSW_XGEN_ENUMS(RCSW_LOG4CL_MODULES)};

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/
/**
 * \brief Representation of a module for debugging.
 *
 * A module is defined on a per file basis (multiple modules in the same file
 * are disallowed).
 */
struct log4cl_module {
  int64_t id; /* id must be the first field for comparisons to work */
  uint8_t lvl; /* The current debugging level */
  char name[20]; /* The name of the module */
};

/**
 * \brief The debugging framework
 *
 * The list of modules currently enabled is maintained by a linked list, and the
 * framework also contains a default level that can be set so that all future
 * modules will be installed with that level by default.
 */
struct log4cl_frmwk {
  struct llist *modules;
  uint8_t default_lvl;
  bool_t initialized;
};

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
BEGIN_C_DECLS
extern struct log4cl_frmwk log4cl_g;

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
struct log4cl_module* log4cl_mod_query(uint64_t id);

bool_t log4cl_mod_enabled(const struct log4cl_module* module, uint8_t lvl);

/**
 * \brief Initialize Debugging Framework
 *
 * This function is idempotent.
 *
 * \return \ref status_t
 */
status_t log4cl_init(void);

/**
 * \brief Shutdown the debugging framework, deallocating memory.
 *
 * The framework can be re-initialized later without error.
 */
void log4cl_shutdown(void);

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
status_t log4cl_insmod(int64_t id, const char *name);

/**
 * \brief Remove a module from the active list by id. If the module is not in
 * the list success, not failure, is returned.
 *
 * \param id The UUID of the module to remove
 * \return \ref status_t
 */
status_t log4cl_rmmod(int64_t id);

/**
 * \brief Remove a module from the active list by name. If the module is not in
 * the list success, not failure, is returned.
 *
 * \param name The name of the module to remove
 * \return \ref status_t
 */
status_t log4cl_rmmod2(const char *name);

/**
 * \brief Set the debugging level for a module
 *
 * \return \ref status_t
 */
status_t log4cl_mod_lvl_set(int64_t id, uint8_t lvl);

/**
 * \brief Set the default debugging level for the debugging framework.
 *
 * All modules installed after calling this function will have the specified
 * level set by default.
 *
 * \param lvl The new default level (one of LOG4CL_OFF, LOG4CL_E, LOG4CL_W, etc.)
 *
 */
static inline void log4cl_default_lvl_set(uint8_t lvl) {
  log4cl_g.default_lvl = lvl;
}

/**
 * \brief Get the ID of a module from its name
 *
 * \param name The name of the module to retrieve the UUID for
 *
 * \return The ID, or -1 if an error occurred.
 */
int64_t log4cl_mod_id_get(const char* name) RCSW_PURE;

END_C_DECLS
