/**
 * \file dbg.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/common/dbg.h"
#include "rcsw/ds/llist.h"

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
BEGIN_C_DECLS

struct dbg_frmwk dbg_g;

/*******************************************************************************
 * Forward Declarations
 ******************************************************************************/
/**
 * \brief Compare to debug modules
 *
 * \param e1 - The first module
 * \param e2 - The second module
 *
 * \return < 0, 0, or >0, depending
 */
static int dbg_mod_cmp(const void* e1, const void* e2);

/*******************************************************************************
 * API Functions
 ******************************************************************************/
status_t dbg_init(void) {
  if (dbg_g.initialized_) {
    return OK;
  }
  struct ds_params params = {
      .cmpe = dbg_mod_cmp,
      .printe = NULL,
      .elt_size = sizeof(struct dbg_module),
      .max_elts = -1,
      .tag = DS_LLIST,
      .flags = 0,
  };
  dbg_g.modules_ = llist_init(NULL, &params);
  RCSW_CHECK_PTR(dbg_g.modules_);
  dbg_g.default_lvl_ = DBG_N;
  dbg_g.initialized_ = TRUE;
  return OK;

error:
  return ERROR;
} /* dbg_init() */

status_t dbg_insmod(int64_t id, const char* const name) {
  struct dbg_module mod;
  mod.id_ = id;
  strncpy(mod.name_, name, sizeof(mod.name_));
  mod.lvl_ = dbg_g.default_lvl_;
  RCSW_CHECK(NULL == llist_data_query(dbg_g.modules_, &id));
  RCSW_CHECK(OK == llist_append(dbg_g.modules_, &mod));
  return OK;

error:
  return ERROR;
} /* dbg_insmod() */

status_t dbg_rmmod(int64_t id) {
  RCSW_CHECK(OK == llist_remove(dbg_g.modules_, &id));
  return OK;
error:
  return ERROR;
} /* dbg_rmmod() */

status_t dbg_rmmod2(const char* const name) {
  int64_t id = dbg_mod_id_get(name);
  RCSW_CHECK(-1 != id);
  RCSW_CHECK(OK == llist_remove(dbg_g.modules_, &id));

error:
  return ERROR;
} /* dbg_rmmod2() */

status_t dbg_mod_lvl_set(int64_t id, uint8_t lvl) {
  struct dbg_module* mod =
      (struct dbg_module*)llist_data_query(dbg_g.modules_, &id);
  RCSW_CHECK_PTR(mod);
  mod->lvl_ = lvl;
  return OK;

error:
  return ERROR;
} /* dbg_mod_lvl_set() */

int64_t dbg_mod_id_get(const char* const name) {
  LLIST_FOREACH(dbg_g.modules_, next, curr) {
    struct dbg_module* mod = (struct dbg_module*)curr->data;
    if (0 != strncmp(name, mod->name_, sizeof(mod->name_))) {
      return mod->id_;
    }
  }
  return -1;
} /* dbg_mod_id_get() */

void dbg_shutdown(void) { llist_destroy(dbg_g.modules_); } /* dbg_shutdown() */

/*******************************************************************************
 * Static Functions
 ******************************************************************************/
int dbg_mod_cmp(const void* const e1, const void* const e2) {
  if (((const struct dbg_module*)e1)->id_ < ((const struct dbg_module*)e2)->id_) {
    return -1;
  }
  if (((const struct dbg_module*)e1)->id_ > ((const struct dbg_module*)e2)->id_) {
    return 1;
  }
  return 0;
} /* dbg_mod_cmp() */

END_C_DECLS
