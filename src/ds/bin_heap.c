/**
 * @file bin_heap.c
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
#include "rcsw/ds/bin_heap.h"
#include "rcsw/common/dbg.h"
#include "rcsw/common/fpc.h"

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
#define MODULE_ID M_DS_BIN_HEAP

/*******************************************************************************
 * Forward Declarations
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * @brief Sift mth element down to its correct place in heap after a deletion
 * from the heap.
 *
 * @param heap The heap handle.
 * @param m The index of the element to sift.
 */
static void bin_heap_sift_down(struct bin_heap* heap, size_t m);

/**
 * @brief Sift nth element up to correct place in heap after insertion.
 *
 * @param heap The heap handle.
 * @param n The index of the element to sift.
 */
static void bin_heap_sift_up(struct bin_heap* heap, size_t i);

/**
 * @brief Swap two elements in the heap using the temporary slot.
 *
 * @param heap The heap handle.
 * @param i1 Index of element #1
 * @param i2 Index of element #2
 */
static void bin_heap_swap(struct bin_heap* heap, size_t i1, size_t i2);

/*******************************************************************************
 * API Functions
 ******************************************************************************/
struct bin_heap* bin_heap_init(struct bin_heap* bin_heap_in,
                               const struct ds_params* const params) {
  RCSW_FPC_NV(NULL,
            params != NULL,
            params->tag == DS_BIN_HEAP,
            params->max_elts > 0,
            params->el_size > 0,
            params->cmpe != NULL);

  struct bin_heap* heap = NULL;
  if (params->flags & DS_APP_DOMAIN_HANDLE) {
    RCSW_CHECK_PTR(bin_heap_in);
    heap = bin_heap_in;
  } else {
    heap = malloc(sizeof(struct bin_heap));
    RCSW_CHECK_PTR(heap);
  }
  heap->flags = params->flags;

  struct ds_params darray_params = {
      .type = {.da =
                   {
                       .init_size =
                           RCSW_MAX((size_t)1, params->type.bhp.init_size + 1),
                   }},
      .printe = params->printe,
      .cmpe = params->cmpe,
      .el_size = params->el_size,
      /* +1 is for the tmp element at index 0 */
      .max_elts = params->max_elts,
      .tag = DS_DARRAY,
      .elements = params->elements,
      .flags = params->flags};
  darray_params.flags |= DS_APP_DOMAIN_HANDLE;
  darray_params.max_elts += (darray_params.max_elts == -1) ? 0 : 1;
  RCSW_CHECK(NULL != darray_init(&heap->arr, &darray_params));
  RCSW_CHECK(OK == darray_set_n_elts(&heap->arr, 1));

  DBGD("init_size=%zu max_elts=%d el_size=%zu flags=0x%08x\n",
       params->type.bhp.init_size,
       params->max_elts,
       params->el_size,
       params->flags);

  return heap;

error:
  bin_heap_destroy(heap);
  errno = EAGAIN;
  return NULL;
} /* bin_heap_init() */

void bin_heap_destroy(struct bin_heap* heap) {
  RCSW_FPC_V(NULL != heap);
  darray_destroy(&heap->arr);
  if (!(heap->flags & DS_APP_DOMAIN_HANDLE)) {
    free(heap);
  }
} /* bin_heap_destroy() */

status_t bin_heap_insert(struct bin_heap* const heap, const void* const e) {
  RCSW_FPC_NV(ERROR, heap != NULL, e != NULL, !bin_heap_isfull(heap));

  RCSW_CHECK(OK == darray_insert(&heap->arr, e, heap->arr.current));

  /* Sift last element up to its correct position in the heap. */
  bin_heap_sift_up(heap, bin_heap_n_elts(heap));
  return OK;

error:
  return ERROR;
} /* bin_heap_insert() */

status_t bin_heap_make(struct bin_heap* const heap,
                       const void* const data,
                       size_t n_elts) {
  RCSW_FPC_NV(ERROR, NULL != heap, NULL != data, n_elts > 0);

  DBGD("Making heap from %zu %zu-byte elements\n", n_elts, heap->arr.el_size);
  for (size_t i = 0; i < n_elts; ++i) {
    RCSW_CHECK(OK == darray_insert(&heap->arr,
                              (const uint8_t*)data + heap->arr.el_size * i,
                              i + 1));
  } /* for(i..) */
  RCSW_CHECK(OK == darray_set_n_elts(&heap->arr, n_elts + 1));

  /* Find median element, (n / 2) */
  size_t k = (bin_heap_n_elts(heap) / 2) + 1;

  /* Sift each element preceding the median down to its correct position. */
  while (k > 1) {
    k--;
    bin_heap_sift_down(heap, k);
  } /* while() */
  return OK;

error:
  return ERROR;
} /* bin_heap_make() */

status_t bin_heap_extract(struct bin_heap* const heap, void* const e) {
  RCSW_FPC_NV(ERROR, heap != NULL, !bin_heap_isempty(heap));

  if (e) {
    ds_elt_copy(e, darray_data_get(&heap->arr, 1), heap->arr.el_size);
  }

  /* Copy last element to tmp position, and sift down to correct position */
  RCSW_CHECK(OK == darray_remove(&heap->arr,
                            darray_data_get(&heap->arr, 1),
                            darray_n_elts(&heap->arr) - 1));
  bin_heap_sift_down(heap, 1);

  return OK;

error:
  return ERROR;
} /* bin_heap_extract() */

status_t bin_heap_update_key(struct bin_heap* const heap,
                             size_t index,
                             const void* const new_val) {
  RCSW_FPC_NV(ERROR, NULL != heap, index > 0, NULL != new_val);
  RCSW_CHECK(OK == darray_data_set(&heap->arr, index, new_val));
  bin_heap_sift_up(heap, index);

  return OK;

error:
  return ERROR;
} /* bin_heap_update_key() */

status_t bin_heap_delete_key(struct bin_heap* const heap,
                             size_t index,
                             const void* const min_val) {
  RCSW_FPC_NV(ERROR, NULL != heap, index > 0, NULL != min_val);
  RCSW_CHECK(OK == bin_heap_update_key(heap, index, min_val));
  RCSW_CHECK(OK == bin_heap_extract(heap, NULL));
  return OK;

error:
  return ERROR;
} /* bin_heap_delete_key() */

void bin_heap_print(const struct bin_heap* const heap) {
  if (heap == NULL) {
    DPRINTF("Heap: < NULL heap >\n");
    return;
  }
  return darray_print(&heap->arr);
} /* bin_heap_print() */

/*******************************************************************************
 * Static Functions
 ******************************************************************************/
static void bin_heap_sift_down(struct bin_heap* const heap, size_t m) {
  size_t l_child = BIN_HEAP_LCHILD(m);
  size_t r_child = BIN_HEAP_RCHILD(m);
  size_t n_elts = bin_heap_n_elts(heap);
  if (heap->flags & DS_RCSW_MIN_HEAP) {
    size_t smallest = m;
    if (l_child <= n_elts &&
        heap->arr.cmpe(darray_data_get(&heap->arr, l_child),
                       darray_data_get(&heap->arr, smallest)) < 0) {
      smallest = l_child;
    }
    if (r_child <= n_elts &&
        heap->arr.cmpe(darray_data_get(&heap->arr, r_child),
                       darray_data_get(&heap->arr, smallest)) < 0) {
      smallest = r_child;
    }
    DBGV("sift_down: n_elts=%zu largest=%zu m=%zu left=%zu right=%zu\n",
         n_elts,
         smallest,
         m,
         l_child,
         r_child);
    if (smallest != m) {
      bin_heap_swap(heap, m, smallest);
      bin_heap_sift_down(heap, smallest);
    }
  } else {
    size_t largest = m;
    if (l_child <= n_elts &&
        heap->arr.cmpe(darray_data_get(&heap->arr, l_child),
                       darray_data_get(&heap->arr, largest)) > 0) {
      largest = l_child;
    }
    if (r_child <= n_elts &&
        heap->arr.cmpe(darray_data_get(&heap->arr, r_child),
                       darray_data_get(&heap->arr, largest)) > 0) {
      largest = r_child;
    }
    DBGV("sift_down: n_elts=%zu largest=%zu m=%zu left=%zu right=%zu\n",
         n_elts,
         largest,
         m,
         l_child,
         r_child);

    if (largest != m) {
      bin_heap_swap(heap, m, largest);
      bin_heap_sift_down(heap, largest);
    }
  }
} /* bin_heap_sift_down() */

static void bin_heap_sift_up(struct bin_heap* const heap, size_t i) {
  /*
   *  While child has higher priority than parent, replace child with parent.
   *  Set child index to parent.  Get next parent, and repeat until top of
   *  heap is reached.
   */
  if (heap->flags & DS_RCSW_MIN_HEAP) {
    while (i != 0 &&
           heap->arr.cmpe(darray_data_get(&heap->arr, BIN_HEAP_PARENT(i)),
                          darray_data_get(&heap->arr, i)) > 0) {
      bin_heap_swap(heap, i, BIN_HEAP_PARENT(i));
      i = BIN_HEAP_PARENT(i);
    } /* while() */
  } else {
    while (i != 0 &&
           heap->arr.cmpe(darray_data_get(&heap->arr, BIN_HEAP_PARENT(i)),
                          darray_data_get(&heap->arr, i)) < 0) {
      bin_heap_swap(heap, i, BIN_HEAP_PARENT(i));
      i = BIN_HEAP_PARENT(i);
    } /* while() */
  }
} /* bin_heap_sift_up() */

static void bin_heap_swap(struct bin_heap* const heap, size_t i1, size_t i2) {
  /*
   * Don't swap if one of the indices is the tmp element. Only happens edge
   * case when the heap is empty you are adding 1st element and sifting up.
   */
  if (i1 == 0 || i2 == 0) {
    return;
  }
  ds_elt_copy(darray_data_get(&heap->arr, 0),
              darray_data_get(&heap->arr, i1),
              heap->arr.el_size);
  ds_elt_copy(darray_data_get(&heap->arr, i1),
              darray_data_get(&heap->arr, i2),
              heap->arr.el_size);
  ds_elt_copy(darray_data_get(&heap->arr, i2),
              darray_data_get(&heap->arr, 0),
              heap->arr.el_size);
} /* bin_heap_swap() */

END_C_DECLS
