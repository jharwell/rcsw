/**
 * \file
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/iter.h"

#include "rcsw/core/fpc.h"

/*******************************************************************************
 * Public API
 ******************************************************************************/
BEGIN_C_DECLS

struct ds_iterator* ds_iter_init(struct ds_iterator*  iter,
                                 void*                ds,
                                 enum ds_iter_type    type,
                                 const struct ds_ops* ops,
                                 bool_t (*classify)(void* e)) {
  RCSW_FPC_NV(NULL, iter != NULL, ds != NULL, ops != NULL, ops->next != NULL);

  /* Backward iteration requires a prev callback. */
  if (type == ekITER_BACKWARD) {
    RCSW_FPC_NV(NULL, ops->prev != NULL);
  }

  iter->ops       = ops;
  iter->container = ds;
  iter->type      = type;
  iter->classify  = classify;
  /*
   * We do NOT set the cursor--that is the responsibility of the individual
   * DS-specific wrappers
   */
  return iter;
}

void* ds_iter_next(struct ds_iterator* const iter) {
  RCSW_FPC_NV(NULL, iter != NULL);

  void* (*advance)(struct ds_iterator*) =
    (iter->type == ekITER_FORWARD) ? iter->ops->next : iter->ops->prev;

  void* e;
  while ((e = advance(iter)) != NULL) {
    if (iter->classify == NULL || iter->classify(e)) {
      return e;
    }
    /* element did not pass the filter — keep advancing */
  }
  return NULL;
} /* ds_iter_next() */

END_C_DECLS
