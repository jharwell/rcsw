/**
 * \file binheap.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/binheap.h"

#define RCSW_ER_MODNAME "rcsw.ds.binheap"
#define RCSW_ER_MODID ekLOG4CL_DS_BINHEAP
#include "rcsw/er/client.h"
#include "rcsw/common/fpc.h"
#include "rcsw/common/alloc.h"

/*******************************************************************************
 * Forward Declarations
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Sift mth element down to its correct place in heap after a deletion
 * from the heap.
 *
 * \param heap The heap handle.
 * \param m The index of the element to sift.
 */
static void binheap_sift_down(struct binheap* heap, size_t m);

/**
 * \brief Sift nth element up to correct place in heap after insertion.
 *
 * \param heap The heap handle.
 * \param n The index of the element to sift.
 */
static void binheap_sift_up(struct binheap* heap, size_t i);

/**
 * \brief Swap two elements in the heap using the temporary slot.
 *
 * \param heap The heap handle.
 * \param i1 Index of element #1
 * \param i2 Index of element #2
 */
static void binheap_swap(struct binheap* heap, size_t i1, size_t i2);

/*******************************************************************************
 * API Functions
 ******************************************************************************/
struct binheap* binheap_init(struct binheap* heap_in,
                             const struct binheap_params* const params) {
  RCSW_FPC_NV(NULL,
              NULL != params,
              params->max_elts > 0,
              params->elt_size > 0,
              NULL != params->cmpe);
  RCSW_ER_MODULE_INIT();

  struct binheap* heap = rcsw_alloc(heap_in,
                                    sizeof(struct binheap),
                                    params->flags & RCSW_NOALLOC_HANDLE);
  RCSW_CHECK_PTR(heap);

  heap->flags = params->flags;

  struct darray_params dparams = {
    /* +1 is for the tmp element at index 0 */
    .init_size =
    RCSW_MAX((size_t)1, params->init_size + 1),
      .printe = params->printe,
      .cmpe = params->cmpe,
      .elt_size = params->elt_size,
      .max_elts = params->max_elts,
      .elements = params->elements,
      .flags = (params->flags & ~RCSW_NOALLOC_HANDLE)};
  dparams.flags |= RCSW_NOALLOC_HANDLE;
  dparams.max_elts += (dparams.max_elts == -1) ? 0 : 1;

  RCSW_CHECK(NULL != darray_init(&heap->arr, &dparams));
  RCSW_CHECK(OK == darray_set_size(&heap->arr, 1));

  ER_DEBUG("init_size=%zu max_elts=%zu elt_size=%zu flags=0x%08x",
             params->init_size,
             params->max_elts,
             params->elt_size,
             params->flags);

  return heap;

error:
  binheap_destroy(heap);
  errno = EAGAIN;
  return NULL;
} /* binheap_init() */

void binheap_destroy(struct binheap* heap) {
  RCSW_FPC_V(NULL != heap);
  darray_destroy(&heap->arr);

  rcsw_free(heap, heap->flags & RCSW_NOALLOC_HANDLE);
} /* binheap_destroy() */

status_t binheap_insert(struct binheap* const heap, const void* const e) {
  RCSW_FPC_NV(ERROR, heap != NULL, e != NULL, !binheap_isfull(heap));

  RCSW_CHECK(OK == darray_insert(&heap->arr, e, heap->arr.current));

  /* Sift last element up to its correct position in the heap. */
  binheap_sift_up(heap, binheap_size(heap));
  return OK;

error:
  return ERROR;
} /* binheap_insert() */

status_t binheap_make(struct binheap* const heap,
                       const void* const data,
                       size_t n_elts) {
  RCSW_FPC_NV(ERROR, NULL != heap, NULL != data, n_elts > 0);

  ER_DEBUG("Making heap from %zu %zu-byte elements",
             n_elts,
             heap->arr.elt_size);
  for (size_t i = 0; i < n_elts; ++i) {
    RCSW_CHECK(OK == darray_insert(&heap->arr,
                                   (const uint8_t*)data + heap->arr.elt_size * i,
                                   i + 1));
  } /* for(i..) */
  RCSW_CHECK(OK == darray_set_size(&heap->arr, n_elts + 1));
  /* Find median element, (n / 2) */
  size_t k = (binheap_size(heap) / 2) + 1;

  /* Sift each element preceding the median down to its correct position. */
  while (k > 1) {
    k--;
    binheap_sift_down(heap, k);
  } /* while() */
  return OK;

error:
  return ERROR;
} /* binheap_make() */

status_t binheap_extract(struct binheap* const heap, void* const e) {
  RCSW_FPC_NV(ERROR, heap != NULL, !binheap_isempty(heap));

  if (e) {
    ds_elt_copy(e, darray_data_get(&heap->arr, 1), heap->arr.elt_size);
  }

  /* Copy last element to tmp position, and sift down to correct position */
  RCSW_CHECK(OK == darray_remove(&heap->arr,
                                 darray_data_get(&heap->arr, 1),
                                 darray_size(&heap->arr) - 1));
  binheap_sift_down(heap, 1);

  return OK;

error:
  return ERROR;
} /* binheap_extract() */

status_t binheap_update_key(struct binheap* const heap,
                             size_t index,
                             const void* const new_val) {
  RCSW_FPC_NV(ERROR, NULL != heap, index > 0, NULL != new_val);
  RCSW_CHECK(OK == darray_data_set(&heap->arr, index, new_val));
  binheap_sift_up(heap, index);

  return OK;

error:
  return ERROR;
} /* binheap_update_key() */

status_t binheap_delete_key(struct binheap* const heap,
                             size_t index,
                             const void* const minmax) {
  RCSW_FPC_NV(ERROR, NULL != heap, index > 0, NULL != minmax);
  RCSW_CHECK(OK == binheap_update_key(heap, index, minmax));
  RCSW_CHECK(OK == binheap_extract(heap, NULL));
  return OK;

error:
  return ERROR;
} /* binheap_delete_key() */

void binheap_print(const struct binheap* const heap) {
  if (heap == NULL) {
    DPRINTF(RCSW_ER_MODNAME " : < NULL >\n");
    return;
  }
  return darray_print(&heap->arr);
} /* binheap_print() */

/*******************************************************************************
 * Static Functions
 ******************************************************************************/
static void binheap_sift_down(struct binheap* const heap, size_t m) {
  size_t l_child = RCSW_BINHEAP_LCHILD(m);
  size_t r_child = RCSW_BINHEAP_RCHILD(m);
  size_t n_elts = binheap_size(heap);

  if (heap->flags & RCSW_DS_BINHEAP_MIN) {
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
    ER_TRACE("sift_down: n_elts=%zu largest=%zu m=%zu left=%zu right=%zu",
               n_elts,
               smallest,
               m,
               l_child,
               r_child);
    if (smallest != m) {
      binheap_swap(heap, m, smallest);
      binheap_sift_down(heap, smallest);
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
    ER_TRACE("sift_down: n_elts=%zu largest=%zu m=%zu left=%zu right=%zu",
               n_elts,
               largest,
               m,
               l_child,
               r_child);

    if (largest != m) {
      binheap_swap(heap, m, largest);
      binheap_sift_down(heap, largest);
    }
  }
} /* binheap_sift_down() */

static void binheap_sift_up(struct binheap* const heap, size_t i) {
  /*
   *  While child has higher priority than parent, replace child with parent.
   *  Set child index to parent.  Get next parent, and repeat until top of
   *  heap is reached.
   */
  if (heap->flags & RCSW_DS_BINHEAP_MIN) {
    while (i != 0 &&
           heap->arr.cmpe(darray_data_get(&heap->arr, RCSW_BINHEAP_PARENT(i)),
                          darray_data_get(&heap->arr, i)) > 0) {
      binheap_swap(heap, i, RCSW_BINHEAP_PARENT(i));
      i = RCSW_BINHEAP_PARENT(i);
    } /* while() */
  } else {
    while (i != 0 &&
           heap->arr.cmpe(darray_data_get(&heap->arr, RCSW_BINHEAP_PARENT(i)),
                          darray_data_get(&heap->arr, i)) < 0) {
      binheap_swap(heap, i, RCSW_BINHEAP_PARENT(i));
      i = RCSW_BINHEAP_PARENT(i);
    } /* while() */
  }
} /* binheap_sift_up() */

static void binheap_swap(struct binheap* const heap, size_t i1, size_t i2) {
  /*
   * Don't swap if one of the indices is the tmp element. Only happens edge
   * case when the heap is empty you are adding 1st element and sifting up.
   */
  if (i1 == 0 || i2 == 0) {
    return;
  }
  ds_elt_copy(darray_data_get(&heap->arr, 0),
              darray_data_get(&heap->arr, i1),
              heap->arr.elt_size);
  ds_elt_copy(darray_data_get(&heap->arr, i1),
              darray_data_get(&heap->arr, i2),
              heap->arr.elt_size);
  ds_elt_copy(darray_data_get(&heap->arr, i2),
              darray_data_get(&heap->arr, 0),
              heap->arr.elt_size);
} /* binheap_swap() */

END_C_DECLS
