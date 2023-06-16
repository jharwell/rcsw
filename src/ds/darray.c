/**
 * \file darray.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/darray.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rcsw/algorithm/search.h"
#include "rcsw/algorithm/sort.h"
#include "rcsw/common/dbg.h"
#include "rcsw/common/fpc.h"

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
#define MODULE_ID M_DS_DARRAY

/*******************************************************************************
 * Forward Declarations
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Increase the capacity of a darray by a set amount
 *
 * \param arr The darray handle
 * \param size The new size
 *
 * \return \ref status_t.
 */
static status_t darray_extend(struct darray* arr, size_t size);

/**
 * \brief Halve the size of an darray
 *
 * Decreases the capacity of a darray by a set amount. If after decreasing the
 * darray would become size 0, the underlying array is NOT free()ed, and the
 * darray's size is set to 0.
 *
 * This function will always fail if \ref RCSW_DS_NOALLOC_DATA was passed during
 * initialization.
 *
 * \param arr The darray handle
 * \param size The new size
 *
 * \return \ref status_t
 */
static status_t darray_shrink(struct darray* arr, size_t size);

/*******************************************************************************
 * API Functions
 ******************************************************************************/
struct darray* darray_init(struct darray* arr_in,
                           const struct ds_params* const params) {
  RCSW_FPC_NV(NULL,
              params != NULL,
              params->tag == ekRCSW_DS_DARRAY,
              params->elt_size > 0,
              params->max_elts != 0);

  struct darray* arr = NULL;

  if (params->flags & RCSW_DS_NOALLOC_HANDLE) {
    arr = arr_in;
  } else {
    arr = malloc(sizeof(struct darray));
  }
  RCSW_CHECK_PTR(arr);
  arr->flags = params->flags;
  arr->elements = NULL;

  if (params->flags & RCSW_DS_NOALLOC_DATA) {
    RCSW_CHECK_PTR(params->elements);
    RCSW_CHECK(params->max_elts > 0);
    arr->elements = params->elements;
    arr->capacity = params->max_elts;
  } else {
    arr->capacity = params->type.da.init_size;
    arr->elements = calloc(params->type.da.init_size, params->elt_size);
    RCSW_CHECK_PTR(arr->elements);
  }
  arr->max_elts = params->max_elts;

  /*
   * If they do not pass anything as cmpe(), then they cannot pass the
   * KEEP_SORTED flag, for obvious reasons.
   */
  if (params->flags & RCSW_DS_SORTED) {
    RCSW_CHECK_PTR(params->cmpe);
  }

  arr->elt_size = params->elt_size;
  arr->current = 0;

  arr->cmpe = params->cmpe;
  arr->printe = params->printe;
  arr->sorted = FALSE;

  DBGD("Capacity=%zu init_size=%zu max_elts=%d elt_size=%zu flags=0x%08x\n",
       arr->capacity,
       params->type.da.init_size,
       arr->max_elts,
       arr->elt_size,
       arr->flags);
  return arr;

error:
  darray_destroy(arr);
  errno = EAGAIN;
  return NULL;
} /* darray_init() */

void darray_destroy(struct darray* arr) {
  RCSW_FPC_V(NULL != arr);

  /*
   * Make it so the array shows as empty if you try to access it again (which
   * is undefined, but, you know, defensive programming...)
   */
  arr->current = 0;
  if (arr->elements && !(arr->flags & RCSW_DS_NOALLOC_DATA)) {
    free(arr->elements);
    arr->elements = NULL;
  }
  if (!(arr->flags & RCSW_DS_NOALLOC_HANDLE)) {
    free(arr);
  }
} /* darray_destroy() */

status_t darray_clear(struct darray* const arr) {
  RCSW_FPC_NV(ERROR, arr != NULL);

  darray_data_clear(arr);
  arr->current = 0;
  return OK;
} /* darray_clear() */

status_t darray_data_clear(struct darray* const arr) {
  RCSW_FPC_NV(ERROR, arr != NULL);

  memset(arr->elements, 0, arr->current * arr->elt_size);
  return OK;
} /* darray_clear() */

status_t
darray_insert(struct darray* const arr, const void* const e, size_t index) {
  RCSW_FPC_NV(ERROR, arr != NULL, e != NULL, (index <= arr->current));

  /* cannot insert--no space left */
  if (darray_isfull(arr) && arr->max_elts != -1) {
    DBGE("ERROR: Cannot insert element: no space\n");
    errno = ENOSPC;
    return ERROR;
  } else if (arr->current >= arr->capacity) {
    RCSW_CHECK(darray_extend(arr, RCSW_MAX(arr->capacity * 2, (size_t)1)) == OK);
  }

  /*
   * If the list is sorted, or if you want to preserve the relative ordering
   * of items, you need to shift items over when inserting.
   */
  if (arr->flags & RCSW_DS_ORDERED) {
    /* shift all elements between index and end of list over by one */
    for (size_t i = arr->current; i > index; --i) {
      ds_elt_copy(
          darray_data_get(arr, i), darray_data_get(arr, i - 1), arr->elt_size);
    } /* for() */
  } else { /* if not, just move element at index to end of array */
    memmove(darray_data_get(arr, arr->current),
            darray_data_get(arr, index),
            arr->elt_size);
  }

  ds_elt_copy(darray_data_get(arr, index), e, arr->elt_size);

  arr->current++;

  /* re-sort the array if configured to */
  if (arr->flags & RCSW_DS_SORTED) {
    arr->sorted = FALSE;
    darray_sort(arr, QSORT_ITER);
  }
  return OK;

error:
  return ERROR;
} /* darray_insert() */

status_t darray_remove(struct darray* const arr, void* const e, size_t index) {
  RCSW_FPC_NV(ERROR, arr != NULL, index <= darray_n_elts(arr));

  if (e != NULL) {
    darray_idx_serve(arr, e, index);
  }

  if (darray_n_elts(arr) == 1) {
    memset(darray_data_get(arr, index), 0xFFFFFFFF, arr->elt_size);
    arr->current--;
    return OK;
  }

  /*
   * If array is sorted, shift all items AFTER index down by one, otherwise,
   * overwrite removed idx with last item in array (MUCH faster)
   */
  if (arr->flags & RCSW_DS_SORTED) {
    memmove(darray_data_get(arr, index),
            darray_data_get(arr, index + 1),
            (arr->current - 1 - index) * arr->elt_size);
  } else {
    memmove(darray_data_get(arr, index),
            darray_data_get(arr, arr->current - 1),
            arr->elt_size);
  }
  arr->current--;

  /*
   * If the array load factor is below 0.25, then shrink the array, in
   * accordance with O(1) amortized deletions from the array.
   */
  if (arr->current / (float)arr->capacity <= 0.25) {
    if (!(arr->flags & RCSW_DS_NOALLOC_DATA)) {
      RCSW_CHECK(OK == darray_shrink(arr, arr->capacity / 2));
    }
  }
  return OK;

error:
  return ERROR;
} /* darray_remove() */

status_t
darray_idx_serve(const struct darray* const arr, void* const e, size_t index) {
  RCSW_FPC_NV(ERROR, arr != NULL, e != NULL, index <= arr->current);
  memmove(e, darray_data_get(arr, index), arr->elt_size);
  return OK;
} /* darray_index_serve() */

int darray_idx_query(const struct darray* const arr, const void* const e) {
  RCSW_FPC_NV(ERROR, NULL != arr, NULL != e, NULL != arr->cmpe);

  int rval = -1;

  if (arr->sorted) {
    DBGD("Currently sorted: performing binary search\n");
    rval = bsearch_rec(
        arr->elements, e, arr->cmpe, arr->elt_size, 0, arr->current - 1);
  } else {
    for (size_t i = 0; i < arr->current; ++i) {
      if (arr->cmpe(e, darray_data_get(arr, i)) == 0) {
        rval = (int)i;
      }
    }
  }

  return rval;
} /* darray_idx_query() */

void* darray_data_get(const struct darray* const arr, size_t index) {
  RCSW_FPC_NV(NULL, arr != NULL);
  return (arr->elements + (index * arr->elt_size));
} /* darray_data_get() */

status_t darray_data_set(const struct darray* const arr,
                         size_t index,
                         const void* const e) {
  RCSW_FPC_NV(ERROR, NULL != arr, NULL != e);
  return ds_elt_copy(arr->elements + index * arr->elt_size, e, arr->elt_size);
} /* darray_data_set() */

status_t darray_resize(struct darray* const arr, size_t size) {
  RCSW_FPC_NV(ERROR, NULL != arr);
  if (size > arr->capacity) {
    return darray_extend(arr, size);
  }
  if (size < arr->capacity) {
    return darray_shrink(arr, size);
  }
  return OK;
} /* darray_resize() */

void darray_print(const struct darray* const arr) {
  if (arr == NULL) {
    DPRINTF("DARRAY: < NULL dynamic array >\n");
    return;
  }
  if (arr->current == 0) {
    DPRINTF("DARRAY: < Empty dynamic array >\n");
    return;
  }
  if (arr->printe == NULL) {
    DPRINTF("DARRAY: < No print function >\n");
    return;
  }

  for (size_t i = 0; i < arr->current; ++i) {
    arr->printe(darray_data_get(arr, i));
  }
  DPRINTF("\n");
} /* darray_print() */

status_t darray_sort(struct darray* const arr, enum alg_sort_type type) {
  RCSW_FPC_NV(ERROR, NULL != arr, NULL != arr->cmpe);

  /*
   * Lists with 0 or 1 elements or that have the sorted flag set are
   * already sorted
   */
  if (arr->current <= 1 || arr->sorted) {
    DBGD("Already sorted: nothing to do\n");
  } else {
    if (type == QSORT_REC) {
      qsort_rec(arr->elements, 0, arr->current - 1, arr->elt_size, arr->cmpe);
    } else if (type == QSORT_ITER) {
      qsort_iter(arr->elements, arr->current - 1, arr->elt_size, arr->cmpe);
    } else {
      return ERROR; /* bad sort type */
    }

    arr->sorted = TRUE;
  }
  return OK;
} /* darray_sort() */

struct darray* darray_filter(struct darray* const arr,
                             bool_t (*pred)(const void* const e),
                             const struct ds_params* const fparams) {
  RCSW_FPC_NV(NULL, NULL != arr, NULL != pred);

  struct ds_params params = { .type = { .da = { .init_size = 0 } },
                              .cmpe = arr->cmpe,
                              .printe = arr->printe,
                              .elt_size = arr->elt_size,
                              .max_elts = arr->max_elts,
                              .tag = ekRCSW_DS_DARRAY,
                              .flags = (fparams == NULL) ? 0 : fparams->flags,
                              .elements = (fparams == NULL) ? NULL
                                                            : fparams->elements };

  struct darray* farr = darray_init(NULL, &params);
  RCSW_CHECK_PTR(farr);

  size_t n_removed = 0;

  /*
   * Remove all matched elements in place. Note the use of the n_removed
   * parameter as a correction factor to make sure I iterate over all
   * elements, because the size of the original list is changing as we iterate
   * over it.
   */
  for (size_t i = 0; i < arr->current + n_removed; ++i) {
    if (pred(darray_data_get(arr, i - n_removed))) {
      RCSW_CHECK(darray_insert(farr,
                               darray_data_get(arr, i - n_removed),
                               farr->current) == OK);
      RCSW_CHECK(darray_remove(arr, NULL, i - n_removed) == OK);
      ++n_removed;
    }
  } /* for() */

  DBGD("%zu %zu-byte elements filtered out into new list. %zu elements remain "
       "in original list.\n",
       n_removed,
       arr->elt_size,
       arr->current);
  return farr;

error:
  darray_destroy(farr);
  return NULL;
} /* darray_filter() */

struct darray* darray_copy(const struct darray* const arr,
                           const struct ds_params* const cparams) {
  RCSW_FPC_NV(NULL, arr != NULL);

  struct ds_params params = {.type = {.da =
                                          {
                                              .init_size = arr->current,
                                          }},
                             .cmpe = arr->cmpe,
                             .printe = arr->printe,
                             .elt_size = arr->elt_size,
                             .max_elts = arr->max_elts,
                             .tag = ekRCSW_DS_DARRAY,
                             .flags = (cparams == NULL) ? 0 : cparams->flags,
                             .elements =
                                 (cparams == NULL) ? NULL : cparams->elements};

  struct darray* carr = darray_init(NULL, &params);
  RCSW_CHECK_PTR(carr);
  carr->current = arr->current;
  carr->sorted = arr->sorted;

  /*
   * Copy items into new list. Don't use ds_elt_copy(), as that does not work
   * if the source list is empty.
   *
   */
  memcpy(carr->elements, arr->elements, arr->current * arr->elt_size);

  DBGD("%zu %zu-byte elements\n", arr->current, arr->elt_size);
  return carr;

error:
  darray_destroy(carr);
  return NULL;
} /* darray_copy() */

status_t darray_map(struct darray* arr, void (*f)(void* e)) {
  RCSW_FPC_NV(ERROR, arr != NULL, f != NULL);

  for (size_t i = 0; i < arr->current; ++i) {
    f(darray_data_get(arr, i));
  }

  return OK;
} /* darray_map() */

status_t darray_inject(const struct darray* const arr,
                       void (*f)(void* e, void* res),
                       void* result) {
  RCSW_FPC_NV(ERROR, arr != NULL, f != NULL, result != NULL);

  for (size_t i = 0; i < arr->current; ++i) {
    f(darray_data_get(arr, i), result);
  }

  return OK;
} /* darray_inject() */

/*******************************************************************************
 * Static Functions
 ******************************************************************************/
static status_t darray_extend(struct darray* const arr, size_t size) {
  if (arr->flags & RCSW_DS_NOALLOC_DATA) {
    DBGE("ERROR: Cannot extend array: RCSW_DS_NOALLOC_DATA\n");
    errno = EAGAIN;
    return ERROR;
  }

  size_t old_size = size;
  arr->capacity = size;

  /* use tmp var to preserve orignal list in case of failure */
  void* tmp = NULL;
  tmp = realloc(arr->elements, arr->capacity * arr->elt_size);

  RCSW_CHECK_PTR(tmp);
  arr->elements = tmp;

  return OK;

error:
  errno = ENOMEM;
  arr->capacity = old_size;
  return ERROR;
} /* darray_extend() */

static status_t darray_shrink(struct darray* const arr, size_t size) {
  RCSW_FPC_NV(ERROR, arr != NULL);

  size_t old_size = arr->capacity;
  arr->capacity = size;

  if (arr->capacity > 0) {
    RCSW_ER_CHECK(!(arr->flags & RCSW_DS_NOALLOC_DATA),
                  "ERROR: Cannot shrink array: RCSW_DS_NOALLOC_DATA");
    void* tmp = realloc(arr->elements, arr->capacity * arr->elt_size);
    RCSW_CHECK_PTR(tmp);
    arr->elements = tmp;
    arr->current = RCSW_MIN(arr->capacity - 1, arr->current);
  } else { /* the array has become empty--don't free() the array */
    arr->current = 0;
  }

  return OK;

error:
  errno = EAGAIN;
  arr->capacity = old_size;
  return ERROR;
} /* darray_shrink() */

END_C_DECLS
