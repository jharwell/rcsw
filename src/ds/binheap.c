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
#include "rcsw/ds/binheap.h"

#define RCSW_ER_MODNAME "rcsw.ds.binheap"
#define RCSW_ER_MODID LOG4CL_DS_BINHEAP
#include <string.h>

#include "rcsw/core/alloc.h"
#include "rcsw/core/fpc.h"
#include "rcsw/er/client.h"

/*******************************************************************************
 * Private API
 ******************************************************************************/
BEGIN_C_DECLS
/**
 * \brief Swap two elements in the heap using the temporary slot (index 0).
 *
 * \param heap The heap handle.
 * \param i1 Index of element #1
 * \param i2 Index of element #2
 */
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
}

/**
 * \brief Sift mth element down to its correct place in heap after a deletion
 * from the heap.
 *
 * \param heap The heap handle.
 * \param m The index of the element to sift.
 */
static void binheap_sift_down(struct binheap* const heap, size_t m) {
  RCSW_FPC_V(NULL != heap);
  size_t l_child = RCSW_BINHEAP_LCHILD(m);
  size_t r_child = RCSW_BINHEAP_RCHILD(m);
  size_t n_elts  = binheap_size(heap);

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
}

/**
 * \brief Sift nth element up to correct place in heap after insertion.
 *
 * \param heap The heap handle.
 * \param i The index of the element to sift.
 */
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
}

/*******************************************************************************
 * Public API
 ******************************************************************************/
struct binheap* binheap_init(struct binheap*                    heap_in,
                             const struct binheap_config* const config) {
  RCSW_FPC_NV(NULL, NULL != config, config->max_elts > 0, config->elt_size > 0);
  ER_ASSERT(NULL != config->cmpe, "binheap requires cmpe()");
  RCSW_ER_MODULE_INIT();

  struct binheap* heap =
    rcsw_alloc(heap_in,
               sizeof(struct binheap),
               config->flags & (RCSW_NOALLOC_HANDLE | RCSW_ZALLOC));
  RCSW_CHECK_PTR(heap);

  heap->flags = config->flags;

  struct darray_config dconfig = {
    /* +1 is for the tmp element at index 0 */
    .init_size = RCSW_MIN(config->init_size, config->max_elts) + 1,
    .printe    = config->printe,
    .cmpe      = config->cmpe,
    .elt_size  = config->elt_size,
    .max_elts  = (int)config->max_elts,
    .elements  = config->elements,
    /* only allocation flags apply to the backing array: e.g. a user-passed
     * RCSW_DS_SORTED must not make it re-sort the heap */
    .flags =
      (config->flags & (RCSW_NOALLOC_DATA | RCSW_ZALLOC)) | RCSW_NOALLOC_HANDLE};
  dconfig.max_elts += (dconfig.max_elts == -1) ? 0 : 1;

  if (NULL == darray_init(&heap->arr, &dconfig)) {
    /* the array was never initialized: only release the handle */
    rcsw_free(heap, heap->flags & RCSW_NOALLOC_HANDLE);
    return NULL; /* errno set by darray_init() */
  }
  RCSW_CHECK(OK == darray_set_size(&heap->arr, 1));

  /*
   * 2026-01-17 [JRH]: We either have to tell darray to zero all allocated
   * memory (safer, but slower), OR zero the first element in the array, as that
   * is used for comparison during insertion.
   */
  memset(heap->arr.elements, 0, heap->arr.elt_size);

  ER_DEBUG("init_size=%zu max_elts=%zu elt_size=%zu flags=0x%08x",
           config->init_size,
           config->max_elts,
           config->elt_size,
           config->flags);

  return heap;

error:
  binheap_destroy(heap);
  errno = ENOMEM;
  return NULL;
} /* binheap_init() */

void binheap_destroy(struct binheap* heap) {
  RCSW_FPC_V(NULL != heap);
  darray_destroy(&heap->arr);

  rcsw_free(heap, heap->flags & RCSW_NOALLOC_HANDLE);
} /* binheap_destroy() */

status_t binheap_insert(struct binheap* const heap, const void* const e) {
  RCSW_FPC_NV(ERROR, heap != NULL, e != NULL);

  if (binheap_isfull(heap)) {
    errno = ENOSPC;
    return ERROR;
  }
  RCSW_CHECK(OK == darray_insert(&heap->arr, e, heap->arr.current));

  /* Sift last element up to its correct position in the heap. */
  binheap_sift_up(heap, binheap_size(heap));
  return OK;

error:
  return ERROR;
} /* binheap_insert() */

status_t binheap_make(struct binheap* const heap,
                      const void* const     data,
                      size_t                n_elts) {
  RCSW_FPC_NV(ERROR, NULL != heap, NULL != data, n_elts > 0);

  ER_DEBUG("Making heap from %zu %zu-byte elements", n_elts, heap->arr.elt_size);

  /* Append to any existing contents, then re-heapify everything (Floyd) */
  for (size_t i = 0; i < n_elts; ++i) {
    RCSW_CHECK(OK ==
               darray_insert(&heap->arr,
                             (const uint8_t*)data + (heap->arr.elt_size * i),
                             heap->arr.current));
  } /* for(i..) */
  for (size_t k = binheap_size(heap) / 2; k >= 1; --k) {
    binheap_sift_down(heap, k);
  } /* for(k..) */
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
                            size_t                index,
                            const void* const     new_val) {
  RCSW_FPC_NV(ERROR,
              NULL != heap,
              index > 0,
              index <= binheap_size(heap),
              NULL != new_val);
  RCSW_CHECK(OK == darray_data_set(&heap->arr, index, new_val));
  /* the key may have moved either way; at most one of these does work */
  binheap_sift_up(heap, index);
  binheap_sift_down(heap, index);

  return OK;

error:
  return ERROR;
} /* binheap_update_key() */

status_t binheap_delete_key(struct binheap* const heap, size_t index) {
  RCSW_FPC_NV(ERROR, NULL != heap, index > 0, index <= binheap_size(heap));

  size_t last = binheap_size(heap);
  if (index != last) {
    /* move the last element into the hole, then restore heap order */
    RCSW_CHECK(OK == darray_data_set(&heap->arr,
                                     index,
                                     darray_data_get(&heap->arr, last)));
  }
  RCSW_CHECK(OK == darray_remove(&heap->arr, NULL, last));
  if (index < last) {
    binheap_sift_up(heap, index);
    binheap_sift_down(heap, index);
  }
  return OK;

error:
  return ERROR;
} /* binheap_delete_key() */

void binheap_print(const struct binheap* const heap) {
  if (heap == NULL) {
    DPRINTF(RCSW_ER_MODNAME " : < NULL >\n");
    return;
  }
  darray_print(&heap->arr);
} /* binheap_print() */

END_C_DECLS
