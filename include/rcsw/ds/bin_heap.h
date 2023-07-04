/**
 * \file bin_heap.h
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
#define RCSW_BIN_HEAP_LCHILD(i) (2*(i))
#define RCSW_BIN_HEAP_RCHILD(i) (2*(i)+1)
#define RCSW_BIN_HEAP_PARENT(i) ((i)/2)

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/
/**
 * \brief Binary heap structure.
 *
 * A heap in which the smallest/largest element is guaranteed to always be on
 * the top of the heap; other elements can be in any order.
 *
 * Implemented using a binary tree inside a \ref darray.
 */
struct bin_heap {
  /**
   * The underlying array with the actual data.
   *
   */
  struct darray arr;

  /**
   * Configuration flags.
   */
  uint32_t flags;
};

/*******************************************************************************
 * Inline Functions
 ******************************************************************************/
/**
 * \brief Determine if the heap is currently full.
 *
 * \param heap The heap handle.
 *
 * \return \ref bool_t
 */
static inline bool_t bin_heap_isfull(const struct bin_heap* const heap) {
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
static inline bool_t bin_heap_isempty(const struct bin_heap* const heap) {
    RCSW_FPC_NV(false, NULL != heap);
    return (bool_t)(darray_n_elts(&heap->arr) == 1);
}

/**
 * \brief Determine # elements currently in the heap.
 *
 * \param heap The heap handle.
 *
 * \return # elements in heap, or 0 on ERROR.
 */
static inline size_t bin_heap_n_elts(const struct bin_heap* const heap) {
    RCSW_FPC_NV(0, NULL != heap);
    return darray_n_elts(&heap->arr) - 1; /* -1 for tmp element */
}

/**
 * \brief Get the number of empty elements in heap
 *
 * \param heap The heap handle.
 *
 * \return # of free elements
 */
static inline size_t bin_heap_n_free(struct bin_heap * heap) {
    return darray_capacity(&heap->arr) - darray_n_elts(&heap->arr);
}

/**
 * \brief Calculate the # of bytes that the heap will require if \ref
 * RCSW_DS_NOALLOC_DATA is passed to manage a specified # of elements of a
 * specified size.
 *
 * \param max_elts # of desired elements the heap will hold
 * \param elt_size size of elements in bytes
 *
 * \return The total # of bytes the application would need to allocate
 */
static inline size_t bin_heap_element_space(size_t max_elts, size_t elt_size) {
  /* +1 is for the tmp element at index 0 */
  return darray_element_space(max_elts, elt_size) + elt_size;
}

/**
 * \brief Empty the heap, but don't deallocate its data.
 *
 * \param heap The heap handle.
 * \return \ref status_t.
 */
static inline status_t bin_heap_clear(struct bin_heap * heap) {
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
static inline void* bin_heap_peek(const struct bin_heap * heap) {
    RCSW_FPC_NV(NULL, heap != NULL, !bin_heap_isempty(heap));
    return darray_data_get(&heap->arr, 1);
}

static inline size_t bin_heap_height(const struct bin_heap * heap) {
  return (size_t)(log10(bin_heap_n_elts(heap)) / log10(2));
}

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Initialize a heap.
 *
 * \param bin_heap_in The heap handle to be filled (can be NULL if
 *                    \ref RCSW_DS_NOALLOC_HANDLE not passed).
 *
 * \param params Initialization parameters.
 *
 * \return The initialized heap, or NULL if an error occurred.
 */
struct bin_heap *bin_heap_init(struct bin_heap *bin_heap_in,
                               const struct ds_params * params) RCSW_CHECK_RET;

/**
 * \brief Destroy a heap. Any further use of the heap handle after calling this
 * function is undefined.
 *
 * \param heap The heap handle.
 */
void bin_heap_destroy(struct bin_heap *heap);

/**
 * \brief Add element to heap
 *
 * \param heap The heap handle.
 * \param e The element to add.
 *
 * \return OK, if successful, ERROR otherwise.
 */
status_t bin_heap_insert(struct bin_heap * heap, const void * e);

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
status_t bin_heap_make(struct bin_heap * heap, const void* data,
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
status_t bin_heap_extract(struct bin_heap * heap, void * e);


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
status_t bin_heap_delete_key(struct bin_heap* heap, size_t index,
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
status_t bin_heap_update_key(struct bin_heap* heap, size_t index,
                               const void* new_val);

/**
 * \brief Print a heap.
 *
 * \param heap The heap handle.
 */
void bin_heap_print(const struct bin_heap * heap);

END_C_DECLS
