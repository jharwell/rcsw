/**
 * \file rbuffer.h
 * \ingroup ds
 * \brief Implementation of ringbuffer.
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
#include "rcsw/common/fpc.h"

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/
/**
 * \brief Ringbuffer data structure.
 */
struct rbuffer {
    /** For printing an element. Can be NULL. */
    void (*printe)(const void *const e);

    /** For comparing elements. Can be NULL. */
    int (*cmpe)(const void *const e1, const void *const e2);

    struct ds_iterator iter;  /// Iterator for elements.
    uint8_t *elements;  /// The actual data.
    size_t current;     /// Current # of elements in buffer.
    size_t max_elts;    /// Maximum # of elements in buffer.
    size_t el_size;     /// Size of an element in the buffer.
    size_t start;       /// Next element insert index.
    uint32_t flags;     /// Run-time configuration parameters.
};

/*******************************************************************************
 * Macros
 ******************************************************************************/
/**
 * \brief Determine if the ringbuffer is currently full.
 *
 * \param rb The ringbuffer handle.
 *
 * \return \ref bool_t
 */
static inline bool_t rbuffer_isfull(const struct rbuffer* const rb) {
    RCSW_FPC_NV(FALSE, NULL != rb);
    return (bool_t)(rb->current == rb->max_elts);
}

/**
 * \brief Determine if the ringbuffer is currently empty.
 *
 * \param rb The ringbuffer handle.
 *
 * \return \ref bool_t
 */
static inline bool_t rbuffer_isempty(const struct rbuffer* const rb) {
    RCSW_FPC_NV(FALSE, NULL != rb);
    return (bool_t)(rb->current == 0);
}

/**
 * \brief Determine # elements currently in the ringbuffer.
 *
 * \param rb The ringbuffer handle.
 *
 * \return # elements in ringbuffer, or 0 on ERROR.
 */
static inline size_t rbuffer_n_elts(const struct rbuffer* const rb) {
    RCSW_FPC_NV(0, NULL != rb);
    return rb->current;
}

/**
 * \brief Get the capacity of the ringbuffer.
 *
 * \param rb The ringbuffer handle.
 *
 * \return The capacity of the ringbuffer, or 0 on ERROR.
 */
static inline size_t rbuffer_capacity(const struct rbuffer* const rb) {
    RCSW_FPC_NV(0, NULL != rb);
    return rb->max_elts;
}

/**
 * \brief Calculate the # of bytes that the ringbuffer will require if \ref
 * DS_APP_DOMAIN_DATA is passed to manage a specified # of elements of a
 * specified size.
 *
 * \param max_elts # of desired elements the ringbuffer will hold.
 * \param el_size size of elements in bytes.
 *
 * \return The total # of bytes the application would need to allocate.
 */
static inline size_t rbuffer_element_space(size_t max_elts, size_t el_size) {
    return ds_calc_element_space1(max_elts, el_size);
}

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Initialize a ringbuffer.
 *
 * \param rb_in An application allocated handle for the ringbuffer. Can be NULL,
 * depending on if \ref DS_APP_DOMAIN_HANDLE is passed or not.
 *
 * \param params The initialization parameters.
 *
 * \return The initialized ringbuffer, or NULL if an error occurred.
 */
struct rbuffer *rbuffer_init(struct rbuffer *rb_in,
    const struct ds_params * params) RCSW_CHECK_RET;

/**
 * \brief Delete a ringbuffer.
 *
 * Any further reference to the ringbuffer after calling this function is
 * undefined.
 *
 * \param rb The ringbuffer handle.
 */
void rbuffer_destroy(struct rbuffer *rb);

/**
 * \brief Add an item into the ringbuffer.
 *
 * This function adds an item into the ringbuffer. If it is
 * currently full, the new entry will be added, and previous data overwritten,
 * UNLESS the ringbuffer is configured to action like a FIFO.
 *
 * \param rb The ringbuffer handle.
 * \param e The element to add.
 *
 * \return \ref status_t.
 */
status_t rbuffer_add(struct rbuffer * rb, const void * e);

/**
 * \brief Remove the next item from the ringbuffer.
 *
 * The head/tail indexes are modified when the element is removed.
 *
 * \param rb The ringbuffer handle.
 * \param e To be filled with the removed element, if non-NULL.
 *
 * \return \ref status_t.
 */
status_t rbuffer_remove(struct rbuffer * rb, void * e);

/**
 * \brief Get the element in the ringbuffer at the specified index.
 *
 * The ringbuffer is not modified.
 *
 * \param rb The ringbuffer handle.
 * \param key The index.
 * \return The element, or NULL if an error occurred.
 */
void *rbuffer_data_get(const struct rbuffer * rb, size_t key);

/**
 * \brief  Get the index of an element in the ringbuffer.
 *
 * \param rb The ringbuffer handle.
 * \param e The element to attempt to get the index of.
 *
 * \return The index of the first element in the rbuffer that matches according
 * to the compare function, or -1 or ERROR.
 */
int rbuffer_index_query(struct rbuffer * rb, const void * e);

/**
 * \brief Retrieve the first entry from the ringbuffer.
 *
 * The ringbuffer is not modified.
 *
 * \param rb The ringbuffer handle.
 * \param e To be filled with the first element.
 *
 * \return \ref status_t.
 */
status_t rbuffer_serve_front(const struct rbuffer * rb, void * e);

/**
 * \brief Get a reference to the first entry in the ringbuffer.
 *
 * The ringbuffer is not modified.
 *
 * \param rb The ringbuffer handle.
 *
 * \return A reference to the first element, or NULL if no such element or an
 * error occurred.
 */
void* rbuffer_front(const struct rbuffer * rb);

/**
 * \brief Clear a rbuffer, but do not deallocate its memory.
 *
 * \param rb The ringbuffer handle.
 *
 * \return \ref status_t.
 */
status_t rbuffer_clear(struct rbuffer * rb);

/**
 * \brief Apply a function to all elements in the ringbuffer.
 *
 * \param rb The ringbuffer handle.
 * \param f The mapping function, which CAN modify elements.
 *
 * \return \ref status_t.
 */
status_t rbuffer_map(struct rbuffer * rb, void (*f)(void *e));

/**
 * \brief Compute a cumulative SOMETHING using all elements in the
 * ringbuffer.
 *
 * \param rb The ringbuffer handle.
 * \param f The mapping function, which CAN modify elements.
 * \param result The initial result, which is passed to the callback along with
 * each element in the ringbuffer.
 *
 * \return \ref status_t.
 */
status_t rbuffer_inject(struct rbuffer * rb, void (*f)(void *e, void *res),
                        void *result);

/**
 * \brief Print the ringbuffer.
 *
 * \param rb The ringbuffer handle.
 */
void rbuffer_print(struct rbuffer * rb);

END_C_DECLS
