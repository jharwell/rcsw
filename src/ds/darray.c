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
#include "rcsw/ds/darray.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RCSW_ER_MODNAME RCSW_ER_MODNAME_BUILDER("rcsw", "ds", "darray")
#define RCSW_ER_MODID LOG4CL_DS_DARRAY
#include "rcsw/algorithm/search.h"
#include "rcsw/algorithm/sort.h"
#include "rcsw/core/alloc.h"
#include "rcsw/core/fpc.h"
#include "rcsw/ds/iter.h"
#include "rcsw/er/client.h"
#include "rcsw/er/macros.h"

/*******************************************************************************
 * Private API
 ******************************************************************************/
BEGIN_C_DECLS
/**
 * \brief Whether the element storage may be resized (realloc()ed).
 *
 * Never with caller-provided storage, and never when heap allocation is
 * disabled library-wide (RCSW_CONFIG_NOALLOC).
 */
static bool_t darray_resizable(const struct darray* const arr) {
#if defined(RCSW_CONFIG_NOALLOC)
  (void)arr;
  return false;
#else
  return !(arr->flags & RCSW_NOALLOC_DATA);
#endif
}

/**
 * \brief Increase the capacity of a darray by a set amount
 *
 * \param arr The darray handle
 * \param size The new size
 *
 * \return \ref status_t.
 */
static status_t darray_extend(struct darray* const arr, size_t size) {
  if (!darray_resizable(arr)) {
    ER_ERR("Cannot extend array: caller-provided or no-heap storage");
    errno = ENOSPC;
    return ERROR;
  }
  if (size > SIZE_MAX / arr->elt_size) {
    ER_ERR("Cannot extend array to %zu elements: size overflow", size);
    errno = ENOMEM;
    return ERROR;
  }

  /* use tmp var to preserve the original array in case of failure */
  void* tmp = realloc(arr->elements, size * arr->elt_size);
  if (NULL == tmp) {
    errno = ENOMEM;
    return ERROR;
  }
  arr->elements = tmp;
  arr->capacity = size;
  return OK;
} /* darray_extend() */

/**
 * \brief Halve the size of an darray
 *
 * Decreases the capacity of a darray by a set amount. If after decreasing the
 * darray would become size 0, the underlying array is NOT free()ed, and the
 * darray's size is set to 0.
 *
 * This function will always fail if \ref RCSW_NOALLOC_DATA was passed during
 * initialization.
 *
 * \param arr The darray handle
 * \param size The new size
 *
 * \return \ref status_t
 */
static status_t darray_shrink(struct darray* const arr, size_t size) {
  RCSW_FPC_NV(ERROR, arr != NULL);

  if (!darray_resizable(arr)) {
    ER_ERR("Cannot shrink array: caller-provided or no-heap storage");
    errno = EINVAL;
    return ERROR;
  }
  if (0 == size) { /* the array has become empty--don't free() the array */
    arr->capacity = 0;
    arr->current  = 0;
    return OK;
  }

  void* tmp = realloc(arr->elements, size * arr->elt_size);
  if (NULL == tmp) {
    errno = ENOMEM;
    return ERROR;
  }
  arr->elements = tmp;
  arr->capacity = size;
  arr->current  = RCSW_MIN(arr->capacity, arr->current);
  return OK;
} /* darray_shrink() */

static void* darray_iter_next_impl(struct ds_iterator* iter) {
  struct darray* arr = iter->container;
  size_t*        idx = &iter->cursor.idx;

  if (*idx >= arr->current) {
    return NULL;
  }
  return darray_data_get(arr, (*idx)++);
} /* darray_iter_next_impl() */

static void* darray_iter_prev_impl(struct ds_iterator* iter) {
  struct darray* arr = iter->container;
  /*
   * cursor stores (index + 1) so that 0 means "before the first element"
   * without needing a signed type. On init, darray_iter_init() sets it to
   * arr->current so the first prev() call returns element [current-1].
   */
  size_t* idx = &iter->cursor.idx;

  if (*idx == 0) {
    return NULL;
  }
  return darray_data_get(arr, --(*idx));
} /* darray_iter_prev_impl() */

static const struct ds_ops darray_iter_ops = {
  .next = darray_iter_next_impl,
  .prev = darray_iter_prev_impl,
};

/*
 * darray_data_get() asserts index < current, but insert legitimately writes
 * at positions up to current (including current itself for append). Use
 * direct pointer arithmetic here; bounds are already enforced by the
 * index <= current precondition.and the capacity/extend check.
 */
#define DARRAY_RAW(arr_, i_) \
  ((uint8_t*)(arr_)->elements + ((i_) * (arr_)->elt_size))

/*******************************************************************************
 * Public API
 ******************************************************************************/
struct darray* darray_init(struct darray*                    arr_in,
                           const struct darray_config* const params) {
  RCSW_FPC_NV(NULL, params != NULL, params->elt_size > 0, params->max_elts != 0);

  /* Sorted arrays cannot be maintained without a comparator */
  ER_ASSERT(!(params->flags & RCSW_DS_SORTED) || NULL != params->cmpe,
            "RCSW_DS_SORTED requires cmpe()");

  struct darray* arr = NULL;

  if (params->flags & RCSW_NOALLOC_DATA) {
    /* Caller-provided space must have a known, fixed size */
    ER_CHECK(-1 != params->max_elts,
             "RCSW_NOALLOC_DATA requires a bounded max_elts");
    ER_CHECK(NULL != params->elements,
             "RCSW_NOALLOC_DATA requires element space");
  }
  ER_CHECK(
    -1 == params->max_elts || params->init_size <= (size_t)params->max_elts,
    "init_size=%zu exceeds max_elts=%d",
    params->init_size,
    params->max_elts);

  arr = rcsw_alloc(arr_in,
                   sizeof(struct darray),
                   params->flags & (RCSW_NOALLOC_HANDLE | RCSW_ZALLOC));
  if (NULL == arr) {
    errno = ENOMEM;
    return NULL;
  }
  arr->flags    = params->flags;
  arr->elements = NULL;
  arr->elt_size = params->elt_size;
  arr->max_elts = params->max_elts;
  arr->current  = 0;
  arr->cmpe     = params->cmpe;
  arr->printe   = params->printe;
  arr->sorted   = false;

  if (params->flags & RCSW_NOALLOC_DATA) {
    arr->capacity = (size_t)params->max_elts;
    arr->elements = rcsw_alloc(params->elements,
                               arr->capacity * params->elt_size,
                               params->flags & (RCSW_NOALLOC_DATA | RCSW_ZALLOC));
  } else {
    /* It is fine to init with an initial capacity of 0 */
    arr->capacity = params->init_size;
    if (arr->capacity > 0) {
      arr->elements = rcsw_alloc(NULL,
                                 params->init_size * params->elt_size,
                                 params->flags & RCSW_ZALLOC);
      if (NULL == arr->elements) {
        darray_destroy(arr);
        errno = ENOMEM;
        return NULL;
      }
    }
  }

  ER_DEBUG("Capacity=%zu init_size=%zu max_elts=%d elt_size=%zu flags=0x%08x",
           arr->capacity,
           params->init_size,
           arr->max_elts,
           arr->elt_size,
           arr->flags);
  return arr;

error:
  errno = EINVAL;
  return NULL;
} /* darray_init() */

void darray_destroy(struct darray* arr) {
  RCSW_FPC_V(NULL != arr);

  /*
   * Make it so the array shows as empty if you try to access it again (which
   * is undefined, but, you know, defensive programming...)
   */
  arr->current = 0;
  rcsw_free(arr->elements, arr->flags & RCSW_NOALLOC_DATA);
  rcsw_free(arr, arr->flags & RCSW_NOALLOC_HANDLE);
} /* darray_destroy() */

status_t darray_clear(struct darray* const arr) {
  RCSW_FPC_NV(ERROR, arr != NULL);

  darray_data_clear(arr);
  arr->current = 0;
  arr->sorted  = false;
  return OK;
} /* darray_clear() */

status_t darray_data_clear(struct darray* const arr) {
  RCSW_FPC_NV(ERROR, arr != NULL);

  if (arr->current > 0) {
    memset(arr->elements, 0, arr->current * arr->elt_size);
  }
  arr->sorted = false;
  return OK;
} /* darray_data_clear() */

/**
 * \brief Index at which \p e must be inserted to keep a sorted array sorted.
 *
 * Returns the position after any elements equal to \p e, so that equal
 * elements keep their insertion order.
 */
static size_t darray_sorted_pos(const struct darray* const arr,
                                const void* const          e) {
  size_t lo = 0;
  size_t hi = arr->current;
  while (lo < hi) {
    size_t mid = lo + ((hi - lo) / 2);
    if (arr->cmpe(DARRAY_RAW(arr, mid), e) <= 0) {
      lo = mid + 1;
    } else {
      hi = mid;
    }
  }
  return lo;
} /* darray_sorted_pos() */

status_t darray_insert(struct darray* const arr,
                       const void* const    e,
                       size_t               index) {
  RCSW_FPC_NV(ERROR, arr != NULL, e != NULL, (index <= arr->current));

  /* cannot insert--no space left */
  if (darray_isfull(arr)) {
    ER_ERR("Cannot insert element: no space");
    errno = ENOSPC;
    return ERROR;
  }
  if (arr->current >= arr->capacity) {
    size_t new_cap = RCSW_MAX(arr->capacity * 2, (size_t)1);
    if (-1 != arr->max_elts) { /* never grow past the configured bound */
      new_cap = RCSW_MIN(new_cap, (size_t)arr->max_elts);
    }
    RCSW_CHECK(darray_extend(arr, new_cap) == OK);
  }

  if (arr->flags & RCSW_DS_SORTED) {
    /*
     * Keep the array sorted on every insert: binary-search for the position
     * and shift the tail, O(n). The caller's index is ignored, as documented.
     * If the array was disturbed (e.g., via darray_data_set()), append and
     * fully sort instead.
     */
    if (arr->sorted || arr->current == 0) {
      index = darray_sorted_pos(arr, e);
    } else {
      index = arr->current;
    }
    memmove(DARRAY_RAW(arr, index + 1),
            DARRAY_RAW(arr, index),
            (arr->current - index) * arr->elt_size);
    memcpy(DARRAY_RAW(arr, index), e, arr->elt_size);
    arr->current++;
    if (!arr->sorted && arr->current > 1) {
      RCSW_CHECK(OK == darray_sort(arr, EXEC_ITER));
    }
    arr->sorted = true;
    return OK;
  }

  if (arr->flags & RCSW_DS_ORDERED) {
    /* shift all elements between index and end of array over by one */
    memmove(DARRAY_RAW(arr, index + 1),
            DARRAY_RAW(arr, index),
            (arr->current - index) * arr->elt_size);
  } else { /* if not, just move element at index to end of array */
    memmove(DARRAY_RAW(arr, arr->current), DARRAY_RAW(arr, index), arr->elt_size);
  }
  memcpy(DARRAY_RAW(arr, index), e, arr->elt_size);
  arr->current++;
  arr->sorted = false;
  return OK;

error:
  return ERROR;
} /* darray_insert() */

status_t darray_remove(struct darray* const arr, void* const e, size_t index) {
  RCSW_FPC_NV(ERROR, arr != NULL, index < darray_size(arr));

  if (e != NULL) {
    darray_idx_serve(arr, e, index);
  }

  /*
   * If the array is sorted, or relative ordering must be preserved, shift all
   * items AFTER index down by one. Otherwise, overwrite the removed index with
   * the last item in the array (MUCH faster), which breaks any sorted order.
   */
  if (arr->flags & (RCSW_DS_SORTED | RCSW_DS_ORDERED)) {
    memmove(DARRAY_RAW(arr, index),
            DARRAY_RAW(arr, index + 1),
            (arr->current - 1 - index) * arr->elt_size);
  } else if (index != arr->current - 1) {
    memcpy(DARRAY_RAW(arr, index),
           DARRAY_RAW(arr, arr->current - 1),
           arr->elt_size);
    arr->sorted = false;
  }
  arr->current--;

  /*
   * If the array load factor is below 0.25, then shrink the array, in
   * accordance with O(1) amortized deletions from the array.
   */
  if (darray_resizable(arr) && arr->capacity > 0 &&
      (double)(arr->current) / (double)arr->capacity <= 0.25) {
    RCSW_CHECK(OK == darray_shrink(arr, arr->capacity / 2));
  }
  return OK;

error:
  return ERROR;
} /* darray_remove() */

status_t darray_idx_serve(const struct darray* const arr,
                          void* const                e,
                          size_t                     index) {
  RCSW_FPC_NV(ERROR, arr != NULL, e != NULL, index < arr->current);
  memmove(e, darray_data_get(arr, index), arr->elt_size);
  return OK;
} /* darray_index_serve() */

int darray_idx_query(const struct darray* const arr, const void* const e) {
  RCSW_FPC_NV(-1, NULL != arr, NULL != e);
  ER_ASSERT(NULL != arr->cmpe, "darray_idx_query() requires cmpe()");

  if (0 == arr->current) {
    return -1;
  }
  if (arr->sorted) {
    ER_DEBUG("Currently sorted: performing binary search (%zu elements)",
             arr->current);

    return bsearch_rec(arr->elements,
                       e,
                       arr->cmpe,
                       arr->elt_size,
                       0,
                       (int)(arr->current - 1));
  }
  for (size_t i = 0; i < arr->current; ++i) {
    if (arr->cmpe(e, darray_data_get(arr, i)) == 0) {
      return (int)i;
    }
  } /* for(i..) */
  return -1;
} /* darray_idx_query() */

void* darray_data_get(const struct darray* const arr, size_t index) {
  RCSW_FPC_NV(NULL, arr != NULL);
  ER_ASSERT(index < arr->current,
            "Index %zu out of bounds (current=%zu)",
            index,
            arr->current);
  return ((uint8_t*)arr->elements + (index * arr->elt_size));
} /* darray_data_get() */

status_t darray_data_set(struct darray* const arr,
                         size_t               index,
                         const void* const    e) {
  RCSW_FPC_NV(ERROR, NULL != arr, NULL != e, index < arr->current);
  memcpy(DARRAY_RAW(arr, index), e, arr->elt_size);
  arr->sorted = false;
  return OK;
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
    DPRINTF(RCSW_ER_MODNAME " : < NULL >\n");
    return;
  }
  if (arr->current == 0) {
    DPRINTF(RCSW_ER_MODNAME " : < Empty >\n");
    return;
  }
  ER_ASSERT(NULL != arr->printe, "darray_print() requires printe()");

  for (size_t i = 0; i < arr->current; ++i) {
    arr->printe(darray_data_get(arr, i));
  }
  DPRINTF("\n");
} /* darray_print() */

status_t darray_sort(struct darray* const arr, enum exec_type type) {
  RCSW_FPC_NV(ERROR, NULL != arr, EXEC_REC == type || EXEC_ITER == type);
  ER_ASSERT(NULL != arr->cmpe, "darray_sort() requires cmpe()");

  /* Arrays with 0 or 1 elements, or already sorted, need no work */
  if (arr->current <= 1 || arr->sorted) {
    ER_DEBUG("Already sorted: nothing to do (%zu elements)", arr->current);
    arr->sorted = true;
    return OK;
  }
  if (type == EXEC_REC) {
    RCSW_CHECK(OK == qsort_rec(arr->elements,
                               0,
                               (int)arr->current - 1,
                               arr->elt_size,
                               arr->cmpe));
  } else {
    RCSW_CHECK(
      OK ==
      qsort_iter(arr->elements, (int)arr->current - 1, arr->elt_size, arr->cmpe));
  }
  arr->sorted = true;
  return OK;

error:
  return ERROR;
}

struct darray* darray_filter(struct darray* const arr,
                             bool_t (*pred)(const void* const e),
                             uint32_t flags,
                             void*    elements) {
  RCSW_FPC_NV(NULL, NULL != arr, NULL != pred, !(flags & RCSW_NOALLOC_HANDLE));

  struct darray_config params = {.init_size = 0,
                                 .cmpe      = arr->cmpe,
                                 .printe    = arr->printe,
                                 .elt_size  = arr->elt_size,
                                 .max_elts  = arr->max_elts,
                                 .flags     = flags,
                                 .elements  = elements};

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

  ER_DEBUG(
    "%zu %zu-byte elements filtered out into new list. %zu elements remain "
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
                           uint32_t                   flags,
                           void*                      elements) {
  RCSW_FPC_NV(NULL, arr != NULL, !(flags & RCSW_NOALLOC_HANDLE));

  struct darray_config params = {.init_size = arr->current,
                                 .cmpe      = arr->cmpe,
                                 .printe    = arr->printe,
                                 .elt_size  = arr->elt_size,
                                 .max_elts  = arr->max_elts,
                                 .flags     = flags,
                                 .elements  = elements};

  struct darray* carr = darray_init(NULL, &params);
  RCSW_CHECK_PTR(carr);
  carr->current = arr->current;
  carr->sorted  = arr->sorted;

  /*
   * Copy items into new list, if it makes sense to do so. Don't use
   * ds_elt_copy(), as that does not work if the source list is empty.
   */
  if (darray_capacity(carr) > 0) {
    memcpy(carr->elements, arr->elements, arr->current * arr->elt_size);
  }

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
  /* f() may have modified elements */
  arr->sorted = false;
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

struct ds_iterator* darray_iter_init(struct ds_iterator* iter,
                                     struct darray*      arr,
                                     enum ds_iter_type   type,
                                     bool_t (*classify)(void* e)) {
  RCSW_FPC_NV(NULL, iter != NULL, arr != NULL);

  /* Seed cursor BEFORE calling ds_iter_init, which must not overwrite it. */
  iter->cursor.idx = (type == ITER_FORWARD) ? 0 : arr->current;

  return ds_iter_init(iter, arr, type, &darray_iter_ops, classify);
}
END_C_DECLS
