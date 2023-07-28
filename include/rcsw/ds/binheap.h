/**
 * \file binheap.h
 * \ingroup ds
 * \brief Implementation of binary heap using a dynamic array (\ref darray).
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <math.h>

#include "rcsw/ds/ds.h"
#include "rcsw/ds/darray.h"
#include "rcsw/common/fpc.h"

/*******************************************************************************
 * Macro Definitions
 ******************************************************************************/
/**
 * \brief Get the index of the left child of an element in a \ref binheap.
 */
#define RCSW_BINHEAP_LCHILD(i) (2*(i))

/**
 * \brief Get the index of the right child of an element in a \ref binheap.
 */
#define RCSW_BINHEAP_RCHILD(i) (2*(i)+1)

/**
 * \brief Get the index of the parent of an element in a \ref binheap.
 */
#define RCSW_BINHEAP_PARENT(i) ((i)/2)

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/
/**
 * \brief \ref binheap initialization parameters.
 */
struct binheap_params {
  /**
   * For comparing elements. Cannot be NULL.
   */
  int (*cmpe)(const void *const e1, const void *const e2);

  /**
   * For comparing keys associated with elements. Cannot be NULL.
   */
  int (*cmpkey)(const void *const e1, const void *const e2);

  /**
   * For printing an element. Can be NULL. If NULL, you can't use \ref
   * binheap_print().
   */
  void (*printe)(const void *e);

  /**
   * Pointer to application-allocated space for storing the \ref binheap
   * data. Ignored unless \ref RCSW_NOALLOC_DATA is passed.
   */
  uint8_t *elements;

  /**
   * Size of elements in bytes.
   */
  size_t elt_size;

  /**
   * Maximum number of elements allowed.
   */
  size_t max_elts;

  /**
   * Configuration flags. See \ref binheap.flags for valid flags.
   */
  uint32_t flags;

  /**
   * Initial size of heap.
   */
  size_t init_size;
};

/**
 * \brief Binary heap structure.
 *
 * A heap in which the smallest/largest element is guaranteed to always be on
 * the top of the heap; other elements can be in any order.
 *
 * Implemented using a binary tree inside a \ref darray.
 */
struct binheap {
  /**
   * The underlying array with the actual data.
   *
   */
  struct darray arr;

  /**
   * Configuration flags. Valid flags are:
   *
   * - \ref RCSW_NOALLOC_HANDLE
   * - \ref RCSW_NOALLOC_DATA
   * - \ref RCSW_DS_BINHEAP_MIN
   * - \ref RCSW_ZALLOC
   *
   * All other flags are ignored.
   */
  uint32_t flags;
};

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Determine if the heap is currently full.
 *
 * \param heap The heap handle.
 *
 * \return \ref bool_t
 */
static inline bool_t binheap_isfull(const struct binheap* const heap) {
    RCSW_FPC_NV(false, NULL != heap);
    return darray_isfull(&heap->arr);
}

/**
 * \brief Determine if the heap is currently empty.
 *
 * \param heap The heap handle.
 *
 * \return \ref bool_t
 */
static inline bool_t binheap_isempty(const struct binheap* const heap) {
    RCSW_FPC_NV(false, NULL != heap);
    return (darray_size(&heap->arr) == 1);
}

/**
 * \brief Determine # elements currently in the heap.
 *
 * \param heap The heap handle.
 *
 * \return # elements in heap, or 0 on ERROR.
 */
static inline size_t binheap_size(const struct binheap* const heap) {
    RCSW_FPC_NV(0, NULL != heap);
    return darray_size(&heap->arr) - 1; /* -1 for tmp element */
}

/**
 * \brief Get the number of empty elements in heap
 *
 * \param heap The heap handle.
 *
 * \return # of free elements
 */
static inline size_t binheap_n_free(struct binheap * heap) {
    return darray_capacity(&heap->arr) - darray_size(&heap->arr);
}

/**
 * \brief Calculate the # of bytes that the heap will require if \ref
 * RCSW_NOALLOC_DATA is passed to manage a specified # of elements of a
 * specified size.
 *
 * \param max_elts # of desired elements the heap will hold
 * \param elt_size size of elements in bytes
 *
 * \return The total # of bytes the application would need to allocate
 */
static inline size_t binheap_element_space(size_t max_elts, size_t elt_size) {
  /* +1 is for the tmp element at index 0 */
  return darray_element_space(max_elts, elt_size) + elt_size;
}

/**
 * \brief Empty the heap, but don't deallocate its data.
 *
 * \param heap The heap handle.
 * \return \ref status_t.
 */
static inline status_t binheap_clear(struct binheap * heap) {
    RCSW_FPC_NV(ERROR, heap != NULL);
    return darray_clear(&heap->arr);
}

/**
 * \brief Get an element from the heap without removing it.
 *
 * \param heap The heap handle.
 *
 * \return Reference to top element on heap, or NULL if an error occurred.
 */
static inline void* binheap_peek(const struct binheap * heap) {
    RCSW_FPC_NV(NULL, heap != NULL, !binheap_isempty(heap));
    return darray_data_get(&heap->arr, 1);
}

/**
 * \brief Get the current height of a \ref binheap
 */
static inline size_t binheap_height(const struct binheap * heap) {
  return (size_t)(log10(binheap_size(heap)) / log10(2));
}

/**
 * \brief Initialize a heap.
 *
 * \param heap_in The heap handle to be filled (can be NULL if
 *                    \ref RCSW_NOALLOC_HANDLE not passed).
 *
 * \param params Initialization parameters.
 *
 * \return The initialized heap, or NULL if an error occurred.
 */
struct binheap *binheap_init(struct binheap *heap_in,
                             const struct binheap_params * params) RCSW_CHECK_RET;

/**
 * \brief Destroy a heap. Any further use of the heap handle after calling this
 * function is undefined.
 *
 * \param heap The heap handle.
 */
void binheap_destroy(struct binheap *heap);

/**
 * \brief Add element to heap
 *
 * \param heap The heap handle.
 * \param e The element to add.
 *
 * \return OK, if successful, ERROR otherwise.
 */
status_t binheap_insert(struct binheap * heap, const void * e);

/**
 * \brief Convert an unordered array into a heap (presumably after you have
 * copied a bunch of data into the heap array, instead of adding elements one at
 * a time). This is O(n), rather than than the O(nlogn) for adding each element
 * one at a time.
 *
 * \param heap The heap handle.
 * \param data A sequential array of data to turn into a heap.
 * \param n_elts # of elements in the data array.
 *
 * \return \ref status_t
 */
status_t binheap_make(struct binheap * heap, const void* data,
                       size_t n_elts);

/**
 * \brief Remove top element from heap.
 *
 * The heap is rebuilt after the element is removed.
 *
 * \param heap The heap handle.
 * \param e The top element, to be filled.
 *
 * \return \ref status_t.
 */
status_t binheap_extract(struct binheap * heap, void * e);


/**
 * \brief Delete the key at index i on the heap.
 *
 * \param heap The heap handle.
 *
 * \param index The index to delete.
 *
 * \param minmax The minimum/maximum value of whatever data type the heap is
 *               managing (i.e. for a min heap of ints it would be
 *               INT_INT). Will be a sentinel data in the allocated array for
 *               the heap.
 *
 * \return \ref status_t.
 */
status_t binheap_delete_key(struct binheap* heap, size_t index,
                             const void* minmax);

/**
 * \brief Update the value of key at index i (presumably a decrease, but it
 * doesn't have to be).
 *
 * \param heap The heap handle.
 * \param index The index to update.
 * \param new_val The new key.
 *
 * \return \ref status_t.
 */
status_t binheap_update_key(struct binheap* heap, size_t index,
                               const void* new_val);

/**
 * \brief Print a heap.
 *
 * \param heap The heap handle.
 */
void binheap_print(const struct binheap * heap);

END_C_DECLS
