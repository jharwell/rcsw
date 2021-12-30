/**
 * @file ds_iter.c
 *
 * @copyright 2017 John Harwell, All rights reserved.
 *
 * This file is part of RCSW.
 *
 * RCSW is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * RCSW is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * RCSW.  If not, see <http://www.gnu.org/licenses/
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/common/fpc.h"
#include "rcsw/ds/darray.h"
#include "rcsw/ds/ds.h"
#include "rcsw/ds/llist.h"
#include "rcsw/ds/rbuffer.h"

/*******************************************************************************
 * Forward Declarations
 ******************************************************************************/
BEGIN_C_DECLS

/*******************************************************************************
 * API Functions
 ******************************************************************************/
struct ds_iterator* ds_iter_init(enum ds_tag tag,
                                 void* const ds,
                                 bool_t (*f)(void* e)) {
  RCSW_FPC_NV(NULL, tag != DS_HASHMAP, tag != DS_BSTREE, ds != NULL);

  struct llist* list = NULL;
  struct darray* arr = NULL;
  struct rbuffer* rb = NULL;
  switch (tag) {
    case DS_DARRAY:
      arr = ds;
      arr->iter.arr = arr;
      arr->iter.index = 0;
      arr->iter.classify = f;
      arr->iter.tag = DS_DARRAY;
      return &arr->iter;
    case DS_LLIST:
      list = ds;
      list->iter.list = list;
      list->iter.curr = list->first;
      list->iter.classify = f;
      list->iter.tag = DS_LLIST;
      return &list->iter;
    case DS_RBUFFER:
      rb = ds;
      rb->iter.rb = rb;
      rb->iter.index = rb->start;
      rb->iter.classify = f;
      rb->iter.tag = DS_RBUFFER;
      return &rb->iter;
    case DS_HASHMAP:
    case DS_BSTREE:
    case DS_FIFO:
    case DS_BIN_HEAP:
    case DS_ADJ_MATRIX:
    case DS_STATIC_MATRIX:
    case DS_DYNAMIC_MATRIX:
    default:
      break;
  } /* switch() */

  return NULL;
} /* ds_iter_init() */

void* ds_iter_next(struct ds_iterator* const iter) {
  RCSW_FPC_NV(NULL, iter != NULL);
  switch (iter->tag) {
    case DS_LLIST: {
      LLIST_ITER(iter->list, iter->curr, next, curr) {
        if (iter->classify && !iter->classify(curr->data)) {
          continue;
        }
        iter->curr = curr->next;
        return curr->data;
      } /* LLIST_ITER() */
    } break;
    case DS_DARRAY:
      while (iter->index < iter->arr->current) {
        if (iter->classify &&
            !iter->classify(darray_data_get(iter->arr, iter->index))) {
          iter->index++;
          continue;
        }
        return darray_data_get(iter->arr, iter->index++);
      } /* while() */
      break;
    case DS_RBUFFER:
      while (iter->index < iter->rb->current) {
        if (iter->classify &&
            !iter->classify(rbuffer_data_get(iter->rb,
                                             (iter->rb->start + iter->index) %
                                                 iter->rb->max_elts))) {
          iter->index++;
          continue;
        }
        return rbuffer_data_get(
            iter->rb, (iter->rb->start + iter->index++) % iter->rb->max_elts);
      } /* while() */
      break;
    case DS_HASHMAP:
    case DS_BSTREE:
    case DS_FIFO:
    case DS_BIN_HEAP:
    case DS_ADJ_MATRIX:
    case DS_STATIC_MATRIX:
    case DS_DYNAMIC_MATRIX:
    default:
      break;
  } /* switch() */

  return NULL;
} /* ds_iter_next() */

END_C_DECLS
