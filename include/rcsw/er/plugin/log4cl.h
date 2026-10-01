/**
 * \file
 *
 * \copyright 2017 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * \ingroup er
 *
 * \brief A C debugging/logging framework in the style of log4c, but less
 * complex.
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdio.h>  // IWYU pragma: keep

#include "rcsw/al/types.h"
#include "rcsw/core/compilers.h"
#include "rcsw/core/core.h"
#include "rcsw/ds/llist.h"

/*******************************************************************************
 * RCSW ER Plugin Definitions
 ******************************************************************************/
/* \cond INTERNAL */

#define RCSW_ER_PLUGIN_PRINTF printf
#define RCSW_ER_PLUGIN_MODNAME_COMPONENT_SEPARATOR "."

#define RCSW_ER_PLUGIN_INIT(...) log4cl_init(__VA_ARGS__)
#define RCSW_ER_PLUGIN_DEINIT(...) log4cl_deinit(__VA_ARGS__)

#define RCSW_ER_PLUGIN_REPORT(LVL, HANDLE, ID, NAME, MSG, ...)               \
  {                                                                          \
    RCSW_ER_PLUGIN_PRINTF(NAME " [" RCSW_XSTR(LVL) "] " MSG, ##__VA_ARGS__); \
  }

#define RCSW_ER_PLUGIN_INSMOD(ID, NAME) log4cl_insmod((ID), (NAME))

#define RCSW_ER_PLUGIN_HANDLE(ID, NAME) log4cl_mod_query(ID)

#define RCSW_ER_PLUGIN_LVL_CHECK(HANDLE, LVL) \
  log4cl_mod_emit(HANDLE, RCSW_JOIN(RCSW_ERL_, LVL))

/* \endcond */

/** \brief Maximum length of a module name, including the NUL; longer names
 * are truncated. */
#define RCSW_LOG4CL_NAMELEN 32

/*******************************************************************************
 * Module codes
 ******************************************************************************/
/**
 * \brief The LOG4CL module codes used by RCSW.
 *
 * When defining your own module codes, you should always start them at or
 * above \c LOG4CL_EXTERNAL, so as to not conflict with the internal codes in
 * RCSW. See \rcswdoc{concepts/event-reporting/ids}.
 */
#define RCSW_LOG4CL_MODULES                                                     \
  LOG4CL_SELF, LOG4CL_DS_BSTREE, LOG4CL_DS_DARRAY, LOG4CL_DS_LLIST,             \
    LOG4CL_DS_HASHMAP, LOG4CL_DS_RBUFFER, LOG4CL_MT_PCQUEUE, LOG4CL_MT_MPOOL,   \
    LOG4CL_SWBUS, LOG4CL_STDIO, LOG4CL_GRIND, LOG4CL_DS_BINHEAP,                \
    LOG4CL_DS_CSMATRIX, LOG4CL_DS_FIFO, LOG4CL_DS_MULTIFIFO, LOG4CL_DS_RAWFIFO, \
    LOG4CL_DS_RBTREE, LOG4CL_TESTING, LOG4CL_DS_OSTREE, LOG4CL_DS_ADJMATRIX,    \
    LOG4CL_DS_MATRIX, LOG4CL_DS_DYNMATRIX, LOG4CL_MT_RDWRLOCK, LOG4CL_MT_RADIX, \
    LOG4CL_MULTIPROCESS, LOG4CL_EXTERNAL

/** \brief Module codes generated from \ref RCSW_LOG4CL_MODULES. */
enum log4cl_module_codes { RCSW_XTABLE_SEQ_ENUM(RCSW_LOG4CL_MODULES) };

/*******************************************************************************
 * Types
 ******************************************************************************/
/**
 * \brief Representation of a module in the LOG4CL plugin.
 *
 * A module is defined on a per file basis (multiple modules in the same file
 * are disallowed).
 *
 * Must be packed and aligned to the same size as \ref dptr_t so that casts from
 * \ref llist_node.data are safe on all targets.
 */
struct RCSW_ATTR(packed, aligned(sizeof(dptr_t))) log4cl_module {
  /**
   * ID of the module, unique within the application. See \ref RCSW_ER_MODID.
   */
  int64_t id;

  /**
   * The current reporting level for the module (an \c RCSW_ERL_* value).
   */
  uint8_t lvl;

  /**
   * Name of the module, truncated to fit (see \ref RCSW_LOG4CL_NAMELEN).
   */
  char name[RCSW_LOG4CL_NAMELEN];
};

/**
 * \brief The LOG4CL plugin.
 *
 * The list of modules currently enabled is maintained by a \ref llist, and the
 * framework also contains a default level that can be set so that all future
 * modules will be installed with that level by default.
 *
 * On POSIX, module installation, removal, lookup, and level changes are
 * serialized internally by a reader/writer lock, so library modules may be
 * initialized from multiple threads concurrently. Reports issued from inside
 * LOG4CL itself (e.g., by the \ref llist that holds the modules) are
 * suppressed.
 */
struct log4cl_plugin {
  struct llist* modules;      ///< The installed \ref log4cl_module objects.
  uint8_t       default_lvl;  ///< Level given to newly installed modules.
  bool_t        initialized;  ///< true between log4cl_init() and log4cl_deinit().
};

/*******************************************************************************
 * Public API
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Check if a module with the specified ID is currently loaded.
 *
 * \param id The module ID.
 *
 * \return The module, or NULL if not found. The pointer stays valid until the
 * module is removed or the plugin is shut down.
 */
RCSW_API struct log4cl_module* log4cl_mod_query(int64_t id);

/**
 * \brief Check if a message with the specified level should be emitted.
 *
 * \param module The module, from \ref log4cl_mod_query(). If NULL, nothing is
 *               emitted.
 * \param lvl The level of the message (an \c RCSW_ERL_* value).
 */
RCSW_API bool_t log4cl_mod_emit(const struct log4cl_module* module,
                                uint8_t                     lvl) RCSW_PURE;

/**
 * \brief Initialize the LOG4CL plugin.
 *
 * This function is idempotent. It must complete before any other thread calls
 * \ref log4cl_insmod().
 *
 * \return \ref status_t
 */
RCSW_API status_t log4cl_init(void);

/**
 * \brief Shut down the LOG4CL plugin.
 *
 * The plugin can be re-initialized later without error. Module handles
 * previously returned by \ref log4cl_mod_query() become invalid, so no other
 * thread may be reporting through LOG4CL when this is called.
 */
RCSW_API void log4cl_deinit(void);

/**
 * \brief Add a module to the active list of debug printing modules.
 *
 * If a module with the same ID is already loaded, nothing is done and OK is
 * returned.
 *
 * \param id The ID of the module to install.
 *
 * \param name The name of the module. Names need not be unique, but should be.
 *             Truncated to \ref RCSW_LOG4CL_NAMELEN - 1 characters.
 *
 * \return \ref status_t
 */
RCSW_API status_t log4cl_insmod(int64_t id, const char* name);

/**
 * \brief Remove a module from the active list by ID.
 *
 * \param id The ID of the module to remove.
 *
 * \return \ref status_t. ERROR with errno=ENOENT if no such module is loaded.
 */
RCSW_API status_t log4cl_rmmod(int64_t id);

/**
 * \brief Remove a module from the active list by name.
 *
 * \param name The name of the module to remove.
 *
 * \return \ref status_t. ERROR with errno=ENOENT if no such module is loaded.
 */
RCSW_API status_t log4cl_rmmod2(const char* name);

/**
 * \brief Set the reporting level for a module.
 *
 * \param id The module ID.
 * \param lvl The new level (an \c RCSW_ERL_* value). See
 *            \rcswdoc{concepts/event-reporting/levels}.
 *
 * \return \ref status_t. ERROR with errno=ENOENT if no such module is loaded.
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
 * \param name The name of the module to retrieve the ID for.
 *
 * \return The ID, or -1 if an error occurred.
 */
RCSW_API int64_t log4cl_mod_id_get(const char* name);

END_C_DECLS
