/**
 * \file
 *
 * \copyright 2017 John Harwell
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/er/plugin/log4cl.h"

#include <assert.h>
#include <errno.h>
#include <string.h>

#include "rcsw/al/al.h"
#if RCSW_CONFIG_PLATFORM == RCSW_CONFIG_PLATFORM_POSIX
#include <pthread.h>
#endif

#include "rcsw/ds/llist.h"
#include "rcsw/er/er.h"

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
BEGIN_C_DECLS

static struct log4cl_plugin g_log4cl;

/*
 * The module list is read on every report and modified by insmod/rmmod, which
 * library modules call from their *_init() functions--possibly concurrently.
 * On POSIX it is protected by a reader/writer lock; on bare metal there is no
 * concurrency to protect against.
 *
 * The list is an llist, and llist reports through ER--i.e., back into
 * log4cl. s_log4cl_busy marks that the current thread is inside log4cl, and
 * log4cl_mod_query() returns NULL while it is set, which suppresses those
 * nested reports instead of re-entering the lock.
 */
#if RCSW_CONFIG_PLATFORM == RCSW_CONFIG_PLATFORM_POSIX
static pthread_rwlock_t     s_log4cl_lock = PTHREAD_RWLOCK_INITIALIZER;
static _Thread_local bool_t s_log4cl_busy = false;
#define LOG4CL_RDLOCK()                          \
  do {                                           \
    (void)pthread_rwlock_rdlock(&s_log4cl_lock); \
    s_log4cl_busy = true;                        \
  } while (0)
#define LOG4CL_WRLOCK()                          \
  do {                                           \
    (void)pthread_rwlock_wrlock(&s_log4cl_lock); \
    s_log4cl_busy = true;                        \
  } while (0)
#define LOG4CL_UNLOCK()                          \
  do {                                           \
    s_log4cl_busy = false;                       \
    (void)pthread_rwlock_unlock(&s_log4cl_lock); \
  } while (0)
#else
static bool_t s_log4cl_busy = false;
#define LOG4CL_RDLOCK() (s_log4cl_busy = true)
#define LOG4CL_WRLOCK() (s_log4cl_busy = true)
#define LOG4CL_UNLOCK() (s_log4cl_busy = false)
#endif

/*******************************************************************************
 * Private API
 ******************************************************************************/
/**
 * \brief Compare two modules by ID.
 *
 * \return < 0, 0, or >0, depending
 */
static int log4cl_mod_cmp(const void* const e1, const void* const e2) {
  if (((const struct log4cl_module*)e1)->id <
      ((const struct log4cl_module*)e2)->id) {
    return -1;
  }
  if (((const struct log4cl_module*)e1)->id >
      ((const struct log4cl_module*)e2)->id) {
    return 1;
  }
  return 0;
} /* log4cl_mod_cmp() */

/**
 * \brief Look up a module by name. Caller must hold the lock.
 */
static struct log4cl_module* log4cl_mod_find_name(const char* const name) {
  if (NULL == g_log4cl.modules) {
    return NULL;
  }
  LLIST_FOREACH(g_log4cl.modules, next, curr) {
    struct log4cl_module* mod = (struct log4cl_module*)curr->data;
    if (0 == strncmp(name, mod->name, sizeof(mod->name))) {
      return mod;
    }
  }
  return NULL;
} /* log4cl_mod_find_name() */

/**
 * \brief Remove the module matching \p key. Caller must hold the write lock.
 */
static status_t log4cl_mod_remove(const struct log4cl_module* const key) {
  return llist_remove(g_log4cl.modules, key);
} /* log4cl_mod_remove() */

/*******************************************************************************
 * Public API
 ******************************************************************************/
status_t log4cl_init(void) {
  status_t rstat = OK;
  LOG4CL_WRLOCK();
  if (!g_log4cl.initialized) {
    struct llist_config params = {
      .cmpe     = log4cl_mod_cmp,
      .printe   = NULL,
      .elt_size = sizeof(struct log4cl_module),
      .max_elts = -1,
      .flags    = 0,
    };
    g_log4cl.modules = llist_init(NULL, &params);
    if (NULL == g_log4cl.modules) {
      rstat = ERROR;
    } else {
      g_log4cl.default_lvl = RCSW_ERL_INFO;
      g_log4cl.initialized = true;
    }
  }
  LOG4CL_UNLOCK();
  return rstat;
} /* log4cl_init() */

status_t log4cl_insmod(int64_t id, const char* const name) {
  RCSW_FPC_NV(ERROR, g_log4cl.initialized, NULL != name);

  struct log4cl_module mod;
  memset(&mod, 0, sizeof(mod));
  mod.id = id;
  (void)snprintf(mod.name, sizeof(mod.name), "%s", name);

  status_t rstat = OK;
  LOG4CL_WRLOCK();
  mod.lvl = g_log4cl.default_lvl;
  /* the comparator takes a struct log4cl_module, not a bare ID */
  if (NULL == llist_data_query(g_log4cl.modules, &mod)) {
    rstat = llist_append(g_log4cl.modules, &mod);
  } /* else already installed */
  LOG4CL_UNLOCK();
  return rstat;
} /* log4cl_insmod() */

struct log4cl_module* log4cl_mod_query(int64_t id) {
  if (s_log4cl_busy) {
    return NULL; /* nested report from inside log4cl: suppress */
  }
  struct log4cl_module* mod = NULL;
  LOG4CL_RDLOCK();
  if (NULL != g_log4cl.modules) {
    struct log4cl_module key = {.id = id};
    mod                      = llist_data_query(g_log4cl.modules, &key);
  }
  LOG4CL_UNLOCK();
  return mod;
} /* log4cl_mod_query() */

bool_t log4cl_mod_emit(const struct log4cl_module* module, uint8_t lvl) {
  if (module) {
    return module->lvl >= lvl;
  }
  return false;
} /* log4cl_mod_emit() */

status_t log4cl_rmmod(int64_t id) {
  RCSW_FPC_NV(ERROR, g_log4cl.initialized);

  struct log4cl_module key = {.id = id};
  LOG4CL_WRLOCK();
  status_t rstat = log4cl_mod_remove(&key);
  LOG4CL_UNLOCK();
  return rstat;
} /* log4cl_rmmod() */

status_t log4cl_rmmod2(const char* const name) {
  RCSW_FPC_NV(ERROR, g_log4cl.initialized, NULL != name);

  status_t rstat = ERROR;
  LOG4CL_WRLOCK();
  struct log4cl_module* mod = log4cl_mod_find_name(name);
  if (NULL == mod) {
    errno = ENOENT;
  } else {
    struct log4cl_module key = {.id = mod->id};
    rstat                    = log4cl_mod_remove(&key);
  }
  LOG4CL_UNLOCK();
  return rstat;
} /* log4cl_rmmod2() */

status_t log4cl_mod_lvl_set(int64_t id, uint8_t lvl) {
  RCSW_FPC_NV(ERROR, g_log4cl.initialized);

  status_t             rstat = ERROR;
  struct log4cl_module key   = {.id = id};
  LOG4CL_WRLOCK();
  struct log4cl_module* mod = llist_data_query(g_log4cl.modules, &key);
  if (NULL == mod) {
    errno = ENOENT;
  } else {
    mod->lvl = lvl;
    rstat    = OK;
  }
  LOG4CL_UNLOCK();
  return rstat;
} /* log4cl_mod_lvl_set() */

void log4cl_default_lvl_set(uint8_t lvl) {
  LOG4CL_WRLOCK();
  g_log4cl.default_lvl = lvl;
  LOG4CL_UNLOCK();
} /* log4cl_default_lvl_set() */

int64_t log4cl_mod_id_get(const char* const name) {
  RCSW_FPC_NV(-1, NULL != name);

  int64_t id = -1;
  LOG4CL_RDLOCK();
  struct log4cl_module* mod = log4cl_mod_find_name(name);
  if (NULL != mod) {
    id = mod->id;
  }
  LOG4CL_UNLOCK();
  return id;
} /* log4cl_mod_id_get() */

void log4cl_deinit(void) {
  LOG4CL_WRLOCK();
  if (g_log4cl.initialized) {
    assert(NULL != g_log4cl.modules && "NULL initialized module list?");
    llist_destroy(g_log4cl.modules);
    g_log4cl.modules     = NULL;
    g_log4cl.initialized = false;
  }
  LOG4CL_UNLOCK();
} /* log4cl_shutdown() */

END_C_DECLS
