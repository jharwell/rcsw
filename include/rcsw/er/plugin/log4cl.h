/**
 * \file log4cl.h
 * \ingroup er
 *
 * \brief A C debugging/logging framework in the style of log4c, but less
 * complex.
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
/* \cond INTERNAL */

#define RCSW_ER_PLUGIN_PRINTF printf
#define RCSW_ER_PLUGIN_MODNAME_COMPONENT_SEPARATOR "."

#define RCSW_ER_PLUGIN_INIT(...) log4cl_init(__VA_ARGS__)
#define RCSW_ER_PLUGIN_DEINIT(...) log4cl_shutdown(__VA_ARGS__)

#define RCSW_ER_PLUGIN_REPORT(LVL, HANDLE,  ID, NAME, MSG, ...) \
  {                                                             \
    RCSW_ER_PLUGIN_PRINTF(NAME " [" RCSW_XSTR(LVL) "] "  MSG,   \
                          ## __VA_ARGS__);                      \
  }

#define RCSW_ER_PLUGIN_INSMOD(ID, NAME) log4cl_insmod((ID), (NAME))

#define RCSW_ER_PLUGIN_HANDLE(ID, NAME) log4cl_mod_query(ID)

#define RCSW_ER_PLUGIN_LVL_CHECK(HANDLE, LVL)           \
  log4cl_mod_emit(HANDLE, RCSW_JOIN(RCSW_ERL_, LVL))

#define RCSW_LOG4CL_NAMELEN 32

/*******************************************************************************
 * Module codes
 ******************************************************************************/
/**
 * \brief The LOG4CL module codes used by RCSW.
 *
 * When defining your own module codes, you should always start them with
 * ekLOG4CL_EXTERNAL, so as to not conflict with the internal codes in RCSW.
 */
#define RCSW_LOG4CL_MODULES                     \
  ekLOG4CL_SELF,                                \
    ekLOG4CL_DS_BSTREE,                         \
    ekLOG4CL_DS_DARRAY,                         \
    ekLOG4CL_DS_LLIST,                          \
    ekLOG4CL_DS_HASHMAP,                        \
    ekLOG4CL_DS_RBUFFER,                        \
    ekLOG4CL_MT_PCQUEUE,                        \
    ekLOG4CL_MT_MPOOL,                          \
    ekLOG4CL_UTILS,                             \
    ekLOG4CL_PULSE,                             \
    ekLOG4CL_STDIO,                             \
    ekLOG4CL_GRIND,                             \
    ekLOG4CL_DS_CORE,                           \
    ekLOG4CL_DS_BINHEAP,                        \
    ekLOG4CL_DS_CSMATRIX,                       \
    ekLOG4CL_COMMON,                            \
    ekLOG4CL_DS_FIFO,                           \
    ekLOG4CL_DS_RAWFIFO,                        \
    ekLOG4CL_ALGORITHM,                         \
    ekLOG4CL_DS_RBTREE,                         \
    ekLOG4CL_TESTING,                           \
    ekLOG4CL_DS_INT_TREE,                       \
    ekLOG4CL_DS_OSTREE,                         \
    ekLOG4CL_DS_ADJ_MATRIX,                     \
    ekLOG4CL_DS_MATRIX,                         \
    ekLOG4CL_DS_DYN_MATRIX,                     \
    ekLOG4CL_MT_RDWRLOCK,                       \
    ekLOG4CL_MT_RADIX,                          \
    ekLOG4CL_MULTIPROCESS,                      \
    ekLOG4CL_CTRL_PID,                          \
    ekLOG4CL_EXTERNAL

enum log4cl_module_codes {RCSW_XTABLE_SEQ_ENUM(RCSW_LOG4CL_MODULES)};
/* \endcond */

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/
/**
 * \brief Representation of a module in the LOG4CL plugin.
 *
 * A module is defined on a per file basis (multiple modules in the same file
 * are disallowed).
 *
 * Must be packed and aligned to the same size as \ref dptr_t so that casts from
 * \ref llist_node.data are same on all targets.
 */
struct RCSW_ATTR(packed, aligned (sizeof(dptr_t))) log4cl_module {
  /**
   * UUID for the module. Must
   *
   * Be the first field for comparisons to work
   */
  int64_t id;

  /**
   * The current reporting level for the module.
   */
  uint8_t lvl;

  /**
   * name of the module.
   */
  char name[RCSW_LOG4CL_NAMELEN];
};

/**
 * \brief The LOG4CL plugin.
 *
 * The list of modules currently enabled is maintained by a \ref llist, and the
 * framework also contains a default level that can be set so that all future
 * modules will be installed with that level by default.
 */
struct log4cl_plugin {
  struct llist *modules;
  uint8_t default_lvl;
  bool_t initialized;
};

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
BEGIN_C_DECLS

/*******************************************************************************
 * API Functions
 ******************************************************************************/
/**
 * \brief Check if a module with the specified ID is currently loaded.
 *
 * \return The module, or NULL if not found.
 */
RCSW_API struct log4cl_module* log4cl_mod_query(uint64_t id) RCSW_CONST;

/**
 * \brief Check if a message with the specified level should be emitted.
 */
RCSW_API bool_t log4cl_mod_emit(const struct log4cl_module* module,
                                uint8_t lvl) RCSW_PURE;

/**
 * \brief Initialize LOG4CL plugin
 *
 * This function is idempotent.
 *
 * \return \ref status_t
 */
RCSW_API status_t log4cl_init(void);

/**
 * \brief Shutdown LOG4CL plugin.
 *
 * The plugin can be re-initialized later without error.
 */
RCSW_API void log4cl_shutdown(void);

/**
 * \brief Add a module to the active list of debug printing modules
 *
 * If the module already exists, ERROR is returned.
 *
 * \param id A UUID for the module to be installed
 *
 * \param name The name of the debugging module. Names do not necessarily have
 *             to be unique within the application, though it's a good idea to
 *             make them that way.
 *
 * \return \ref status_t
 */
RCSW_API status_t log4cl_insmod(int64_t id, const char *name);

/**
 * \brief Remove a module from the active list by ID.
 *
 * If the module is not in the list success, not failure, is returned.
 *
 * \param id The UUID of the module to remove
 *
 * \return \ref status_t
 */
RCSW_API status_t log4cl_rmmod(int64_t id);

/**
 * \brief Remove a module from the active list by name.
 *
 * If the module is not in the list success, not failure, is returned.
 *
 * \param name The name of the module to remove
 * \return \ref status_t
 */
RCSW_API status_t log4cl_rmmod2(const char *name);

/**
 * \brief Set the reporting level for a module
 *
 * \return \ref status_t
 */
RCSW_API status_t log4cl_mod_lvl_set(int64_t id, uint8_t lvl);

/**
 * \brief Set the default reporting level for the plugin.
 *
 * All modules installed after calling this function will have the specified
 * level set by default.
 *
 * \param lvl The new default level.
 */
RCSW_API void log4cl_default_lvl_set(uint8_t lvl);

/**
 * \brief Get the ID of a module from its name.
 *
 * \param name The name of the module to retrieve the UUID for.
 *
 * \return The ID, or -1 if an error occurred.
 */
RCSW_API int64_t log4cl_mod_id_get(const char* name) RCSW_PURE;

END_C_DECLS
