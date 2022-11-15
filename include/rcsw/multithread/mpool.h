/**
 * \brief mpool.h
 * \ingroup multithread
 * \brief Implementation of memory/buffer pool of memory chunks.
 *
 * Sort of a malloc() approximation.
 *
 * \copyright 2017 John Harwell, All rights reserved.
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

#ifndef INCLUDE_RCSW_MULTITHREAD_MPOOL_H_
#define INCLUDE_RCSW_MULTITHREAD_MPOOL_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/multithread/mt_csem.h"
#include "rcsw/multithread/mt_mutex.h"
#include "rcsw/ds/ds.h"
#include "rcsw/ds/llist.h"

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
/**
 * Passing this flag will cause the pool to keep track of how many things are
 * using a given piece of memory, so that \ref mpool_release() will only release
 * the memory after the reference count reaches 0.
 *
 * If you do not specify this flag you cannot rely on the results of \ref
 * mpool_ref_query(), and \ref mpool_ref_add()/\ref mpool_ref_remove() cannot be
 * used.
 */
#define MPOOL_REF_COUNT_EN DS_EXT_FLAGS

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/
/**
 * \brief Memory pool queue initialization parameters
 */
struct mpool_params {
    size_t el_size;     /// Size of each element in bytes.
    size_t max_elts;    /// # of elements the pool will hold.
    uint8_t *nodes;     /// Space for linked list nodes.
    uint8_t *elements;  /// Space for the actual elements.

    /**
     * Initialization flags. You can pass any of the following  DS flags:
     * \ref DS_APP_DOMAIN_DATA, \ref DS_APP_DOMAIN_NODES, \ref
     * DS_APP_DOMAIN_DATA, individually or in combination, or you can pass \ref
     * MT_APP_DOMAIN_MEM which is equivalent to all 3.
     */
    uint32_t flags;
};

/**
 * \brief Memory pool structure (a threadsafe malloc()/free() over a set of
 * memory chunks of a fixed size).
 */
struct mpool {
    uint8_t *elements;   /// The actual elements.
    uint8_t *nodes;      /// Space for the llist nodes.
    struct llist free;   /// Pool free list.
    struct llist alloc;  /// Pool allocated list.

    int *refs;         /// Pointer to array for reference counting
    size_t n_free;     /// # elements in the pool currently free.
    size_t n_alloc;    /// # elements in the pool currently allocated.
    size_t el_size;    /// Size of elements in the pool in bytes.
    size_t max_elts;   /// Max # of elements in the pool.
    mt_csem_t sem;     /// Semaphore used for waiting on free slots in pool.
    mt_mutex_t mutex;  /// Mutex used to protect mpool integrity.
    uint32_t flags;    /// Run time configuration flags.
};

/*******************************************************************************
 * Macros
 ******************************************************************************/
/**
 * \brief Get # of bytes needed for space for the mpool nodes.
 *
 * \param max_elts # of desired elements in pool.
 *
 * \return The # of bytes the application would need to allocate.
 */
static inline size_t  mpool_node_space(size_t max_elts) {
    /* x2 for free and alloc lists */
    return 2 * llist_node_space(max_elts);
}

/**
 * \brief Get # of bytes needed for space for the mpool data.
 *
 * \param max_elts # of desired elements in pool.
 * \param el_size Size of elements in bytes.
 *
 * \return The # of bytes the application would need to allocate.
 */
static inline size_t  mpool_element_space(size_t max_elts, size_t el_size) {
    return ds_calc_element_space2(max_elts, el_size);
}


/**
 * \brief Determine if the memory pool is currently full.
 *
 * \param pool The memory pool handle.
 *
 * \return \ref bool_t
 */
static inline bool_t mpool_isfull(const struct mpool* const pool) {
    RCSW_FPC_NV(FALSE, NULL != pool);
    return llist_isfull(&pool->free);
}

/**
 * \brief Determine if the memory pool is currently empty.
 *
 * \param pool The pool handle.
 *
 * \return \ref bool_t
 */
static inline bool_t mpool_isempty(const struct mpool* const pool) {
    RCSW_FPC_NV(FALSE, NULL != pool);
    return llist_isempty(&pool->alloc);
}

/**
 * \brief Determine # elements currently in the memory pool.
 *
 * \param pool The pool handle.
 *
 * \return # elements in memory pool, or 0 on ERROR.
 */
static inline size_t mpool_n_elts(const struct mpool* const pool) {
    RCSW_FPC_NV(0, NULL != pool);
    return llist_n_elts(&pool->alloc) + llist_n_elts(&pool->free);
}

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Initialize a memory pool.
 *
 * \param pool_in An application allocated handle for the memory pool. Can be
 * NULL, depending on if \ref DS_APP_DOMAIN_HANDLE is passed or not.
 * \param params The initialization parameters.
 *
 * \return The initialized pool, or NULL if an error occurred.
 */
struct mpool*mpool_init(struct mpool * pool_in,
                        const struct mpool_params * params) RCSW_CHECK_RET;

/**
 * \brief Deallocate a memory pool. Any further use of the pool handle after
 * calling this function is undefined.
 *
 * \param the_pool The mpool handle.
 */
void mpool_destroy(struct mpool * the_pool);

/**
 * \brief Request a memory from a pool. If no memory of the requested type is
 * current available, wait until some becomes available.
 *
 * \param the_pool The mpool handle.
 *
 * \return The allocated chunk, or NULL if an error occurred.
 */
uint8_t *mpool_req(struct mpool * the_pool);

/**
 * \brief Release allocated memory from a pool (presumably after you have
 * finished using it).
 *
 * Note that if \ref MPOOL_REF_COUNT_EN was passed, then this function will not
 * actually free memory until the last reference has released it.
 *
 * \param the_pool The mpool handle.
 * \param ptr The memory to release.
 *
 * \return \ref status_t.
 */
status_t mpool_release(struct mpool * the_pool, uint8_t * ptr);

/**
 * \brief Add a reference to a chunk of memory (must have been previously requested).
 *
 * \param the_pool The mpool handle.
 * \param ptr The chunk to add a reference to.
 *
 * \return \ref status_t.
 */
status_t mpool_ref_add(struct mpool * the_pool, const uint8_t * ptr);

/**
 * \brief Remove a reference to a chunk of memory (must have been previously requested).
 *
 * \param the_pool The mpool handle.
 * \param ptr The chunk to remove a reference from.
 *
 * \return \ref status_t
 */
status_t mpool_ref_remove(struct mpool * the_pool,
                          const uint8_t * ptr);

/**
 * \brief Get the reference count of an allocated chunk. This function does NOT
 * perform locking, so you need to lock at a higher level if you want to be able
 * to rely on the value returned.
 *
 * \param the_pool The mpool handle.
 * \param ptr The chunk to query.
 *
 * \return The reference count, or -1 on error.
 */
int mpool_ref_query(struct mpool * the_pool, const uint8_t* ptr);

END_C_DECLS

#endif /* INCLUDE_RCSW_MULTITHREAD_MPOOL_H_  */
