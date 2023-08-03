/**
 * \file darray.h
 * \ingroup ds
 * \brief Implementation of dynamic array (i.e., std::vector).
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/ds.h"
#include "rcsw/ds/iter.h"
#include "rcsw/algorithm/sort.h"
#include "rcsw/common/fpc.h"

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/
/**
 * \brief Dynamic array (darray) initialization parameters.
 */
struct darray_params {
  /**
   * For comparing elements. Can be NULL. If NULL, sorting the \ref darray is
   * disabled.
   */
  int (*cmpe)(const void *const e1, const void *const e2);

  /**
   * For printing an element. Can be NULL. If NULL, you can't use \ref
   * darray_print().
   */
  void (*printe)(const void *e);

  /**
   * Pointer to application-allocated space for storing data managed by the \ref
   * darray. Ignored unless \ref RCSW_NOALLOC_DATA is passed.
   */
  dptr_t *elements;

  /**
   * Size of elements in bytes.
   */
  size_t elt_size;

  /**
   * Maximum number of elements allowed. -1 = no upper limit.
   */
  int max_elts;

  /**
   * Configuration flags. See \ref darray.flags for valid flags.
   */
  uint32_t flags;

  /**
   * Initial size of the array (must be < max_elts). Ignored if \ref
   * RCSW_NOALLOC_DATA is passed.
   */
  size_t init_size;
};

/**
 * \brief Dynamic array data structure.
 *
 * It follows the dynamic table resizing algorithm from
 * _Introduction_to_Algorithms, in order to provide O(1) amortized insertions
 * and deletions. You can also resize the darray to whatever size you want at
 * any time.
 */
struct darray {
  /** The array that holds the actual data */
  uint8_t *elements;

  struct ds_iterator iter;

  /**
   * Next item in the darray will be inserted here (also # of elements in
   * darray).
   */
  size_t current;

  /**
   * Current capacity of array in # slots
   */
  size_t capacity;

  /**
   * Size in bytes of an element
   **/
  size_t elt_size;

  /**
   * Is the array is currently sorted?
   */
  bool_t sorted;

  /**
   * Maximum number of elements allowed in the array (-1 for no limit)
   */
  int max_elts;

  /**
   * \brief Run-time configuration flags.
   *
   * Valid flags are:
   *
   * - \ref RCSW_ZALLOC
   * - \ref RCSW_NOALLOC_HANDLE
   * - \ref RCSW_NOALLOC_DATA
   * - \ref RCSW_DS_SORTED
   *
   * All other flags are ignored.
   */
  uint32_t flags;

  /**
   * For comparing elements for <,=,>. Can be NULL. If NULL, certain darray
   * operations aren't allowed (like sorting).
   */
  int (*cmpe)(const void *const e1, const void *const e2);

  /** For printing an element (can be NULL) */
  void (*printe)(const void *e);
};

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Determine if the dynamic array is currently full
 *
 * \param arr The dynamic array handle
 *
 * \return \ref bool_t
 */
static inline bool_t darray_isfull(const struct darray* const arr) {
    RCSW_FPC_NV(false, NULL != arr);
    return (arr->current == (size_t)arr->max_elts);
}

/**
 * \brief Determine if the dynamic arrayis currently empty
 *
 * \param arr The dynamic array handle
 *
 * \return \ref bool_t
 */
static inline bool_t darray_isempty(const struct darray* const arr) {
    RCSW_FPC_NV(false, NULL != arr);
    return arr->current == 0;
}

/**
 * \brief Determine # elements currently in the \ref darray.
 *
 * \param arr The dynamic array handle
 *
 * \return # elements in arr, or 0 on ERROR
 */
static inline size_t darray_size(const struct darray* const arr) {
    RCSW_FPC_NV(0, NULL != arr);
    return arr->current;
}

/**
 * \brief Determine arr capacity.
 *
 * \param arr The dynamic array handle.
 *
 * \return # elements in arr, or 0 on ERROR.
 */
static inline size_t darray_capacity(const struct darray* const arr) {
    RCSW_FPC_NV(0, NULL != arr);
    return arr->capacity;
}

/**
 * \brief Set # elements currently in the \ref darray.
 *
 * You can't use this function to the # elements to greater than the current
 * capacity, even if the max # of elements for the array is greater. Use \ref
 * darray_resize() if you need that.
 *
 * \param arr The dynamic array handle.
 * \param n_elts The new # of elements in the array.
 * \return \ref status_t.
 */
static inline status_t darray_set_size(struct darray* const arr,
                                         size_t n_elts) {
    RCSW_FPC_NV(ERROR, NULL != arr, n_elts <= arr->capacity);
    arr->current = n_elts;
    return OK;
}

/**
 * \brief Calculate the # of bytes that will be required for the specified # of
 * elements and the element size.
 *
 * \param max_elts # of desired elements the dynamic array will hold.
 * \param elt_size size of elements in bytes.
 *
 * \return The total # of bytes the application would need to allocate.
 */
static inline size_t darray_element_space(size_t max_elts, size_t elt_size) {
    return ds_elt_space_simple(max_elts, elt_size);
}

/**
 * \brief Initialize an darray
 *
 * It is valid to initialize the darray with an initial size of 0.
 *
* \param arr_in The handle to be filled. Must be non-NULL if \ref
 *                RCSW_NOALLOC_HANDLE passed in \ref darray_params.flags.
 *
 * \param params Initialization parameters.
 *
 * \return The initialized list, or NULL if an ERROR occurred
 */
RCSW_API struct darray *darray_init(struct darray *arr_in,
                           const struct darray_params * params) RCSW_CHECK_RET;

/**
 * \brief Delete a darray
 *
 * Any use of the darray handle after calling this function is undefined.
 *
 * \param arr The darray handle
 */
RCSW_API void darray_destroy(struct darray *arr);

/**
 * \brief Clear a darray
 *
 * Resets the count of # elements in array to 0, and memset()s all elements to 0.
 *
 * \param arr The darray handle
 *
 * \return \ref status_t
 */
RCSW_API status_t darray_clear(struct darray * arr);

/**
 * \brief Clear a darray's data (the element count is not affected)
 *
 * Memset()s all elements to 0.
 *
 * \param arr The darray handle
 *
 * \return \ref status_t
 */
RCSW_API status_t darray_data_clear(struct darray * arr);

/**
 * \brief Insert an item into an darray
 *
 * Inserts an item into the given position in the darray. If the array currently
 * full, it will be extended to accomodate the new element, unless doing so
 * would cause the max_elts for the darray to be exceeded. If \ref
 * RCSW_DS_SORTED was passed during initialization, the darray is resorted after
 * insertion.
 *
 * If the array was initialized with an unlimited maximum # of elements, then
 * the array is extended to twice its current size (for amortized O(1)
 * insertions).
 *
 * \param arr The darray handle
 * \param e The element to insert
 * \param index The index to insert at. If so, this function behaves as a
 * prepend function (very inefficient, but allowable). If equal to
 * \ref darray_size(), behaves as append function (efficient--use this).
 *
 * \return \ref status_t
 */
RCSW_API status_t darray_insert(struct darray * arr, const void * e,
                       size_t index);
/**
 * \brief Remove an item from an darray
 *
 * If \ref RCSW_DS_SORTED was passed during initialization, then the remaining
 * elements in the list are shifted to close the empty slot and preserve the
 * sort order (inefficient). If \ref RCSW_DS_SORTED was not passed during
 * initialization, then the last element in the array is simply used to
 * overwrite the element to be removed, which is a much faster operation.
 *
 * If the darray was initialized with an unlimited capacity, then the darray is
 * shrunk to 1/2 its current size whenever the current deletion brought its
 * utilization down at or below 25% (for amortized O(1) deletions).
 *
 * \param arr The darray handle
 * \param e To be filled with the removed element if non-NULL
 * \param index The index of the element to remove
 *
 * \return \ref status_t
 */
RCSW_API status_t darray_remove(struct darray * arr, void * e, size_t index);

/**
 * \brief Get an element in a darray (array not modified)
 *
 * \param arr The darray handle
 * \param e To be filled with the served element
 * \param index Index of element to get
 *
 * \return \ref status_t
 */
RCSW_API status_t darray_idx_serve(const struct darray * arr, void * e,
                            size_t index);

/**
 * \brief Find the index of an element
 *
 * Find the first occurence of the element in the darray which is is equal to
 * e. A recursive implementation of binary search will be used if the darray is
 * sorted, linear scan otherwise. This function can only be called if \ref
 * darray.cmpe was non-NULL during initialization.
 *
 * \param arr The darray handle
 * \param e To be filled with the served element
 *
 * \return The index, or -1 if not found
 */
RCSW_API int darray_idx_query(const struct darray * arr, const void * e);

/**
 * \brief Retrieve an item from an darray
 *
 * Returns the item at the specified index. There is NO upper bounds checking to
 * make sure that index < \ref darray_size().
 *
 * \param arr The darray handle
 * \param index The index to get the data for
 *
 * \return: The element, or NULL if an error occurred
 */
RCSW_API void *darray_data_get(const struct darray * arr, size_t index);

/**
 * \brief Set an item in a darray
 *
 * Sets the item at the specified index to the passed in value. There is NO
 * upper bounds checking to  make sure that index < \ref darray_size().
 *
 * \param arr The darray handle
 * \param index The index of the item to set
 * \param e The new value
 *
 * \return \ref status_t
 */
RCSW_API status_t darray_data_set(const struct darray* arr, size_t index,
                         const void* e);

/**
 * \brief Resize a darray
 *
 * Resizes the darray to the specified size. If size < current capacity of the
 * array, then the array is reduced to its first "size" elements. If size >
 * current capacity, then the array is increased to be able to hold "size" elements.
 *
 * \param arr The darray handle.
 * \param size The new size.
 *
 * \return \ref status_t.
 */
RCSW_API status_t darray_resize(struct darray * arr, size_t size);

/**
 * \brief Copy the data from one darray to another.
 *
 * The element sizes for the two arrays must be the same. The capacities can be
 * different, but the destination must have enough space for the source. The
 * data from the source is placed starting at the beginning of the destination
 * array. The source and destination array cannot overlap.
 *
 * \param arr1 The destination.
 * \param arr2 The source.
 *
 * \return \ref status_t.
 */
RCSW_API status_t darray_data_copy(const struct darray* arr1,
                          const struct darray* arr2);
/**
 * \brief Sort a darray
 *
 * Can only be called if \ref darray.cmpe was non-NULL during initialization.
 *
 * \param arr The darray handle
 *
 * \param type \ref exec_type.
 *
 * \return \ref status_t.
 */
RCSW_API status_t darray_sort(struct darray * arr, enum exec_type type);

/**
 * \brief Apply a function to all elements in the darray.
 *
 * \param arr The darray handle.
 * \param f The callback for each element (can modify elements).
 *
 * \return \ref status_t.
 */
RCSW_API status_t darray_map(struct darray *arr, void (*f)(void *e));

/**
 * \brief Compute a cumulative SOMETHING using all elements in the darray
 *
 * \param arr The darray handle
 * \param f The callback for each element (can modify elements). It is passed
 * each element in turn, along with the in-progress result.
 * \param result The initial result.
 *
 * \return \ref status_t.
 *
 */
RCSW_API status_t darray_inject(const struct darray * arr,
                       void (*f)(void *e, void *res), void *result);

/**
 * \brief Filter out elements into a new darray
 *
 * Iterate through the darray and find all the items that satisfy the predicate,
 * and return a new darray containing only the items for which the predicate
 * returned true. Matched elements are transferred from the original array to
 * the new array, and removed from the original array. If no elements are found
 * that fulfill the predicate, an empty darray is returned.
 *
 * \param arr The darray handle
 * \param pred The predicate
 * \param flags Initialization flags for the new \ref darray
 *
 * \param elements Space for the elements of the new \ref darray. Can be NULL.
 *
 * \return The filtered array, or NULL if an error occured.
 */
RCSW_API struct darray *darray_filter(struct darray * arr,
                             bool_t (*pred)(const void * e),
                             uint32_t flags,
                             void* elements);

/**
 * \brief Create a copy of a darray
 *
 * Makes a copy of an darray by initializing a new darray and copying the
 * original darray's data to it. The original darray is unchanged.
 *
 * \param arr The darray handle.
 *
 * \param flags Initialization flags for the new \ref darray
 *
 * \param elements Space for the elements of the new \ref darray. Can be NULL.
 *
 * \return The new arr, or NULL if an error occurred.
 */
RCSW_API struct darray *darray_copy(const struct darray * arr,
                           uint32_t flags,
                           void* elements);

/**
 * \brief Print an darray
 *
 * \param arr The darray handle
 */
RCSW_API void darray_print(const struct darray * arr);

END_C_DECLS
