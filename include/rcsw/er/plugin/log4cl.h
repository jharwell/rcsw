/**
 * \file log4cl.h
 * \ingroup er
 * \brief A C debugging/logging framework in the style of log4c, but less complex.
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
#define RCSW_ER_PLUGIN_PRINTF printf

#define RCSW_ER_PLUGIN_INIT(...) log4cl_init(__VA_ARGS__)
#define RCSW_ER_PLUGIN_DEINIT(...) log4cl_shutdown(__VA_ARGS__)

#define RCSW_ER_PLUGIN_REPORT(LVL, HANDLE,  ID, NAME, MSG, ...) \
  {                                                             \
    RCSW_ER_PLUGIN_PRINTF(NAME " [" RCSW_XSTR(LVL) "] "  MSG,   \
                          ## __VA_ARGS__);                      \
  }

#define RCSW_ER_PLUGIN_INSMOD(ID, NAME) log4cl_insmod((ID), (NAME))

#define RCSW_ER_PLUGIN_HANDLE(ID, NAME) log4cl_mod_query(ID)

#define RCSW_ER_PLUGIN_LVL_CHECK(HANDLE, LVL) log4cl_mod_enabled(HANDLE, LVL)

#define RCSW_LOG4CL_MODNAME_LEN 32

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
    M_GRIND,                                    \
    M_DS_CORE,                                  \
    M_DS_BINHEAP,                              \
    M_DS_CSMATRIX,                              \
    M_COMMON,                                   \
    M_DS_FIFO,                                  \
    M_DS_RAWFIFO,                               \
    M_ALGORITHM,                                \
    M_DS_RBTREE,                                \
    M_TESTING,                                  \
    M_DS_INT_TREE,                              \
    M_DS_OSTREE,                                \
    M_DS_ADJ_MATRIX,                            \
    M_DS_MATRIX,                                \
    M_DS_DYN_MATRIX,                            \
    M_MT_RDWRLOCK,                              \
    M_MT_RADIX,                                 \
    M_MULTIPROCESS,                             \
    M_CTRL_PID,                                 \
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
  char name[RCSW_LOG4CL_MODNAME_LEN]; /* The name of the module */
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

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
/**
 * \brief Check if a module with the specified ID is currently loaded.
 *
 * \return The module, or NULL if not found.
 */
struct log4cl_module* log4cl_mod_query(uint64_t id) RCSW_CONST;

bool_t log4cl_mod_enabled(const struct log4cl_module* module,
                          uint8_t lvl) RCSW_PURE;

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
void log4cl_default_lvl_set(uint8_t lvl);

/**
 * \brief Get the ID of a module from its name
 *
 * \param name The name of the module to retrieve the UUID for
 *
 * \return The ID, or -1 if an error occurred.
 */
int64_t log4cl_mod_id_get(const char* name) RCSW_PURE;

END_C_DECLS
