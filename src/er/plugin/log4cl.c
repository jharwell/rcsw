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

#include "rcsw/ds/llist.h"

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
BEGIN_C_DECLS

struct log4cl_frmwk log4cl_g;

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
  if (log4cl_g.initialized) {
    return OK;
  }
  struct ds_params params = {
    .cmpe = log4cl_mod_cmp,
    .printe = NULL,
    .elt_size = sizeof(struct log4cl_module),
    .max_elts = -1,
    .tag = ekRCSW_DS_LLIST,
    .flags = 0,
  };
  log4cl_g.modules = llist_init(NULL, &params);
  RCSW_CHECK_PTR(log4cl_g.modules);
  log4cl_g.default_lvl = RCSW_ER_PLUGIN_INFO;
  log4cl_g.initialized = TRUE;
  return OK;

error:
  return ERROR;
} /* log4cl_init() */

status_t log4cl_insmod(int64_t id, const char* const name) {
  RCSW_FPC_NV(ERROR,
              log4cl_g.initialized);

  struct log4cl_module mod;
  mod.id = id;
  strncpy(mod.name, name, sizeof(mod.name));
  mod.lvl = log4cl_g.default_lvl;
  RCSW_CHECK(NULL == llist_data_query(log4cl_g.modules, &id));
  RCSW_CHECK(OK == llist_append(log4cl_g.modules, &mod));
  return OK;

error:
  return ERROR;
} /* log4cl_insmod() */

struct log4cl_module* log4cl_mod_query(uint64_t id) {
  if (log4cl_g.modules) {
    return llist_data_query(log4cl_g.modules, &id);
  }
  return NULL;
}

bool_t log4cl_mod_enabled(const struct log4cl_module* module, uint8_t lvl) {
  if (module)  {
    return module->lvl <= lvl;
  }
  return FALSE;
}
status_t log4cl_rmmod(int64_t id) {
  RCSW_CHECK(OK == llist_remove(log4cl_g.modules, &id));
  return OK;
error:
  return ERROR;
} /* log4cl_rmmod() */

status_t log4cl_rmmod2(const char* const name) {
  int64_t id = log4cl_mod_id_get(name);
  RCSW_CHECK(-1 != id);
  RCSW_CHECK(OK == llist_remove(log4cl_g.modules, &id));

error:
  return ERROR;
} /* log4cl_rmmod2() */

status_t log4cl_mod_lvl_set(int64_t id, uint8_t lvl) {
  struct log4cl_module* mod = llist_data_query(log4cl_g.modules, &id);
  RCSW_CHECK_PTR(mod);
  mod->lvl = lvl;
  return OK;

error:
  return ERROR;
} /* log4cl_mod_lvl_set() */

int64_t log4cl_mod_id_get(const char* const name) {
  LLIST_FOREACH(log4cl_g.modules, next, curr) {
    struct log4cl_module* mod = (struct log4cl_module*)curr->data;
    if (0 != strncmp(name, mod->name, sizeof(mod->name))) {
      return mod->id;
    }
  }
  return -1;
} /* log4cl_mod_id_get() */

void log4cl_shutdown(void) {
  llist_destroy(log4cl_g.modules);
  log4cl_g.modules = NULL;
  log4cl_g.initialized = FALSE;
} /* log4cl_shutdown() */


END_C_DECLS
