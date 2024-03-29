/**
 * \file iter.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/iter.h"

#include "rcsw/common/fpc.h"
#include "rcsw/ds/darray.h"
#include "rcsw/ds/llist.h"
#include "rcsw/ds/rbuffer.h"

/*******************************************************************************
 * Forward Declarations
 ******************************************************************************/
BEGIN_C_DECLS

/*******************************************************************************
 * API Functions
 ******************************************************************************/
struct ds_iterator*
ds_iter_init(void* const ds, enum ds_tag tag, enum ds_iter_type type) {
  RCSW_FPC_NV(NULL, ds != NULL);

  struct ds_iterator* iter = NULL;
  switch (tag) {
    case ekRCSW_DS_DARRAY: {
      struct darray* da = ds;
      iter = &da->iter;
      iter->arr = ds;
      if (ekITER_FORWARD == type) {
        iter->index = 0;
      } else if (ekITER_BACKWARD == type) {
        iter->index = darray_size(da) - 1;
      }
    } break;
    case ekRCSW_DS_LLIST: {
      struct llist* list = ds;
      iter = &list->iter;
      if (ekITER_FORWARD == type) {
        iter->curr = list->first;
      } else if (ekITER_BACKWARD == type) {
        iter->curr = list->last;
      }
      iter->index = 0;
    } break;
    case ekRCSW_DS_RBUFFER: {
      struct rbuffer* rb = ds;
      iter = &rb->iter;
      iter->rb = ds;
      if (ekITER_FORWARD == type) {
        iter->index = rb->start;
      } else if (ekITER_BACKWARD == type) {
        iter->index = -1;
      }
    } break;
    default:
      return NULL;
      break;
  } /* switch() */

  iter->tag = tag;
  iter->type = type;
  iter->classify = NULL;

  return iter;
} /* ds_iter_init() */

struct ds_iterator*
ds_filter_init(void* const ds, enum ds_tag tag, bool_t (*f)(void* e)) {
  RCSW_FPC_NV(NULL, ds != NULL);

  struct ds_iterator* iter = NULL;
  switch (tag) {
    case ekRCSW_DS_DARRAY: {
      struct darray* da = ds;
      iter = &da->iter;
      iter->arr = ds;
      iter->index = 0;
    } break;
    case ekRCSW_DS_LLIST: {
      struct llist* list = ds;
      iter = &list->iter;
      iter->curr = list->first;
      iter->index = 0;
    } break;
    case ekRCSW_DS_RBUFFER: {
      struct rbuffer* rb = ds;
      iter = &rb->iter;
      iter->rb = ds;
      iter->index = rb->start;
    } break;
    default:
      return NULL;
      break;
  } /* switch() */

  iter->classify = f;
  iter->tag = tag;
  iter->type = ekITER_FORWARD;

  return iter;
} /* ds_filter_init() */

void* ds_iter_next(struct ds_iterator* const iter) {
  RCSW_FPC_NV(NULL, iter != NULL);
  switch (iter->tag) {
    case ekRCSW_DS_LLIST: {
      LLIST_ITER(iter->list, iter->curr, next, curr) {
        /* check for filtering */
        if (iter->classify && !iter->classify(curr->data)) {
          continue;
        }

        if (ekITER_FORWARD == iter->type) {
          iter->curr = curr->next;
        } else {
          iter->curr = curr->prev;
        }
        return curr->data;
      } /* LLIST_ITER() */
    } break;
    case ekRCSW_DS_DARRAY:
      if (ekITER_FORWARD == iter->type) {
        while ((size_t)iter->index < iter->arr->current) {
          if (iter->classify &&
              !iter->classify(darray_data_get(iter->arr, iter->index))) {
            iter->index++;
            continue;
          }
          return darray_data_get(iter->arr, iter->index++);
        } /* while() */
      } else {
        while (iter->index >= 0) {
          if (iter->classify &&
              !iter->classify(darray_data_get(iter->arr, iter->index))) {
            iter->index--;
            continue;
          }
          return darray_data_get(iter->arr, iter->index--);
        } /* while() */
      }
      break;
    case ekRCSW_DS_RBUFFER:
      if (ekITER_FORWARD == iter->type) {
        size_t idx = (iter->rb->start + iter->index) % iter->rb->max_elts;

        while ((size_t)iter->index < iter->rb->current) {
          if (iter->classify &&
              !iter->classify(rbuffer_data_get(iter->rb, idx))) {
            iter->index++;
            continue;
          }
          iter->index++;
          return rbuffer_data_get(iter->rb, idx);
        } /* while() */
      } else {
        while (iter->index != (int)iter->rb->start) {
          if (-1 == iter->index) {
            iter->index = iter->rb->start;
          }
          iter->index = (iter->index + rbuffer_size(iter->rb) - 1) %
                        rbuffer_size(iter->rb);
          if (iter->classify &&
              !iter->classify(rbuffer_data_get(iter->rb, iter->index))) {
            continue;
          }
          return rbuffer_data_get(iter->rb, iter->index);
        } /* while() */
      }
      break;
    default:
      break;
  } /* switch() */

  return NULL;
} /* ds_iter_next() */

END_C_DECLS
