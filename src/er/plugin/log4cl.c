/**
 * \file log4cl.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/er/plugin/log4cl.h"
#include "rcsw/er/client.h"

#include "rcsw/ds/llist.h"

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
BEGIN_C_DECLS

struct log4cl_frmwk g_log4cl;

/*******************************************************************************
 * Private Functions
 ******************************************************************************/
/**
 * \brief Compare to debug modules
 *
 * \param e1 - The first module
 * \param e2 - The second module
 *
 * \return < 0, 0, or >0, depending
 */
static int log4cl_mod_cmp(const void* const e1, const void* const e2) {
  if (((const struct log4cl_module*)e1)->id < ((const struct log4cl_module*)e2)->id) {
    return -1;
  }
  if (((const struct log4cl_module*)e1)->id > ((const struct log4cl_module*)e2)->id) {
    return 1;
  }
  return 0;
} /* log4cl_mod_cmp() */

/*******************************************************************************
 * API Functions
 ******************************************************************************/
status_t log4cl_init(void) {
  if (g_log4cl.initialized) {
    return OK;
  }
  struct llist_params params = {
    .cmpe = log4cl_mod_cmp,
    .printe = NULL,
    .elt_size = sizeof(struct log4cl_module),
    .max_elts = -1,
    .flags = 0,
  };
  g_log4cl.modules = llist_init(NULL, &params);
  RCSW_CHECK_PTR(g_log4cl.modules);
  g_log4cl.default_lvl = RCSW_ERL_INFO;
  g_log4cl.initialized = true;
  return OK;

error:
  return ERROR;
} /* log4cl_init() */

status_t log4cl_insmod(int64_t id, const char* const name) {
  RCSW_FPC_NV(ERROR,
              g_log4cl.initialized);

  struct log4cl_module mod;
  mod.id = id;
  strncpy(mod.name, name, sizeof(mod.name));
  mod.lvl = g_log4cl.default_lvl;
  RCSW_CHECK(NULL == llist_data_query(g_log4cl.modules, &id));
  RCSW_CHECK(OK == llist_append(g_log4cl.modules, &mod));
  return OK;

error:
  return ERROR;
} /* log4cl_insmod() */

struct log4cl_module* log4cl_mod_query(uint64_t id) {
  if (g_log4cl.modules) {
    return llist_data_query(g_log4cl.modules, &id);
  }
  return NULL;
}

bool_t log4cl_mod_enabled(const struct log4cl_module* module, uint8_t lvl) {
  if (module)  {
    return module->lvl <= lvl;
  }
  return false;
}
status_t log4cl_rmmod(int64_t id) {
  RCSW_CHECK(OK == llist_remove(g_log4cl.modules, &id));
  return OK;
error:
  return ERROR;
} /* log4cl_rmmod() */

status_t log4cl_rmmod2(const char* const name) {
  int64_t id = log4cl_mod_id_get(name);
  RCSW_CHECK(-1 != id);
  RCSW_CHECK(OK == llist_remove(g_log4cl.modules, &id));

error:
  return ERROR;
} /* log4cl_rmmod2() */

status_t log4cl_mod_lvl_set(int64_t id, uint8_t lvl) {
  struct log4cl_module* mod = llist_data_query(g_log4cl.modules, &id);
  RCSW_CHECK_PTR(mod);
  mod->lvl = lvl;
  return OK;

error:
  return ERROR;
} /* log4cl_mod_lvl_set() */

void log4cl_default_lvl_set(uint8_t lvl) {
  g_log4cl.default_lvl = lvl;
}

int64_t log4cl_mod_id_get(const char* const name) {
  LLIST_FOREACH(g_log4cl.modules, next, curr) {
    struct log4cl_module* mod = (struct log4cl_module*)curr->data;
    if (0 != strncmp(name, mod->name, sizeof(mod->name))) {
      return mod->id;
    }
  }
  return -1;
} /* log4cl_mod_id_get() */

void log4cl_shutdown(void) {
  llist_destroy(g_log4cl.modules);
  g_log4cl.modules = NULL;
  g_log4cl.initialized = false;
} /* log4cl_shutdown() */


END_C_DECLS
