/**
 * \brief mpool.h
 * \ingroup multithread
 * \brief Implementation of memory/buffer pool of memory chunks.
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/multithread/csem.h"
#include "rcsw/multithread/mutex.h"
#include "rcsw/ds/ds.h"
#include "rcsw/ds/llist.h"

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/
/**
 * \brief Memory pool initialization parameters.
 */
struct mpool_params {
  /**
   * Pointer to application-allocated space for storing the \ref llist_node
   * objects used to carve up \ref mpool_params.elements. Ignored unless \ref
   * RCSW_NOALLOC_META is passed.
   */
  dptr_t *meta;

  /**
   * Pointer to application-allocated space for the pool.Ignored unless \ref
   * RCSW_NOALLOC_META is passed.
   */
  dptr_t *elements;

  /**
   * Size of elements in bytes.
   */
  size_t elt_size;

  /**
   * Number of elements in the pool.
   */
  size_t max_elts;

  /**
   * Configuration flags. See \ref mpool.flags for valid flags.
   */
  uint32_t flags;
};

/**
 * \brief Memory pool: a threadsafe malloc()/free() over a set of memory chunks.
 *
 * Memory chunks must be fixed size.
 */
struct mpool {
  /** The chunks of managed memory. */
  dptr_t           *elements;

  /** Space for the llist nodes for both the free and allocated lists. */
  struct llist_node *nodes;

  /** List of free chunks of memory. */
  struct llist      free;

  /** List of free chunks of memory. */
  struct llist      alloc;

  /** Reference counting. Same length as max # elements. */
  int               *refs;

  /** Size of elements in the pool in bytes. */
  size_t            elt_size;

  /** Max # of elements in the pool. Must be > 0. */
  size_t            max_elts;

  /**
   * Used to wait for a chunk to become free in \ref mpool_req().
   */
  struct csem       slots_avail;

  /** Lock around most operations for concurrency safety. */
  struct mutex      mutex;

  /**
   * Run time configuration flags. Valid flags are:
   *
   * - \ref RCSW_ZALLOC
   * - \ref RCSW_NOALLOC_HANDLE
   * - \ref RCSW_NOALLOC_DATA
   * - \ref RCSW_NOALLOC_META
   *
   * All other flags are ignored.
   */
  uint32_t          flags;
};

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Get # of bytes needed for space for the mpool nodes.
 *
 * \param max_elts # of desired elements in pool.
 *
 * \return The # of bytes the application would need to allocate.
 */
static inline size_t  mpool_meta_space(size_t max_elts) {
    /* x2 for free and alloc lists */
    return 2 * llist_meta_space(max_elts);
}

/**
 * \brief Get # of bytes needed for space for the mpool data.
 *
 * \param max_elts # of desired elements in pool.
 * \param elt_size Size of elements in bytes.
 *
 * \return The # of bytes the application would need to allocate.
 */
static inline size_t  mpool_element_space(size_t max_elts, size_t elt_size) {
    return ds_elt_space_with_meta(max_elts, elt_size);
}


/**
 * \brief Determine if the memory pool is currently full.
 *
 * \note The returned value cannot be relied upon in concurrent contexts without
 * additional synchronization.
 *
 * \param pool The pool handle.
 *
 * \return \ref bool_t
 */
static inline bool_t mpool_isfull(const struct mpool* const pool) {
    RCSW_FPC_NV(false, NULL != pool);
    return llist_isfull(&pool->alloc);
}

/**
 * \brief Determine if the \ref mpool is currently empty.
 *
 * \note The returned value cannot be relied upon in concurrent contexts without
 * additional synchronization.
 *
 * \param pool The pool handle.
 *
 * \return \ref bool_t
 */
static inline bool_t mpool_isempty(const struct mpool* const pool) {
    RCSW_FPC_NV(false, NULL != pool);
    return llist_isempty(&pool->alloc);
}

/**
 * \brief Determine # elements currently in the memory pool.
 *
 * \note The returned value cannot be relied upon in concurrent contexts without
 * additional synchronization.
 *
 * \param pool The pool handle.
 *
 * \return # elements in memory pool, or 0 on ERROR.
 */
static inline size_t mpool_size(const struct mpool* const pool) {
    RCSW_FPC_NV(0, NULL != pool);
    return llist_size(&pool->alloc);
}

/**
 * \brief Determine maximum # elements in the memory pool.
 *
 * \param pool The pool handle.
 *
 * \return Max # elements in memory pool, or 0 on ERROR.
 */
static inline size_t mpool_capacity(const struct mpool* const pool) {
    RCSW_FPC_NV(0, NULL != pool);
    return pool->max_elts;
}


/**
 * \brief Initialize a \ref mpool.
 *
 * \param pool_in An application allocated handle for the memory pool. Can be
 *                NULL, depending on if \ref RCSW_NOALLOC_HANDLE is passed or
 *                not.
 *
 * \param params The initialization parameters.
 *
 * \return The initialized pool, or NULL if an error occurred.
 */
struct mpool*mpool_init(struct mpool * pool_in,
                        const struct mpool_params * params) RCSW_CHECK_RET;

/**
 * \brief Destroy a \ref mpool.
 *
 * Any further use of the pool handle after calling this function is undefined.
 *
 * \param the_pool The mpool handle.
 */
void mpool_destroy(struct mpool * the_pool);

/**
 * \brief Request a memory chunk from a \ref mpool.
 *
 * If no memory of the requested type is currently available, wait indefinitely.
 *
 * \param the_pool The mpool handle.
 *
 * \return The allocated chunk, or NULL if an error occurred.
 */
void *mpool_req(struct mpool * the_pool);

/**
 * \brief Request a memory chunk from a \ref mpool with a timeout.
 *
 * \param the_pool The mpool handle.
 *
 * \param to Timeout.
 *
 * \param chunk The pointer to fill. Can be NULL.
 *
 * \return The allocated chunk, or NULL if an error occurred.
 */
status_t mpool_timedreq(struct mpool * the_pool,
                        const struct timespec* to,
                        void** chunk);

/**
 * \brief Release a chunk of memory from a \ref mpool.
 *
 * Memory chunk will not actually be freed until the last reference has released
 * it.
 *
 * \param the_pool The mpool handle.
 *
 * \param ptr The memory to release.
 *
 * \return \ref status_t.
 */
status_t mpool_release(struct mpool * the_pool, void * ptr);

/**
 * \brief Increment ref count for a previously allocated chunk in a \ref mpool.
 *
 * This function is useful if a thread/module wants to give a non-owning
 * reference to a chunk of memory to another thread/module.
 *
 * \param the_pool The mpool handle.
 *
 * \param ptr The chunk to add a reference to.
 *
 * \return \ref status_t.
 */
status_t mpool_ref_add(struct mpool * the_pool, const void * ptr);

/**
 * \brief Decrement ref count for a currently allocated chunk in a \ref mpool.
 *
 * If the reference count reaches 0 as a result of calling this function, the
 * chunk is not freed.
 *
 * \param the_pool The mpool handle.
 *
 * \param ptr The chunk to remove a reference from.
 *
 * \return \ref status_t
 */
status_t mpool_ref_remove(struct mpool * the_pool,
                          const void * ptr);

/**
 * \brief Get the index of the reference count (not the reference count)
 *
 * \note The returned value cannot be relied upon in concurrent contexts without
 * additional synchronization.
 *
 * \param the_pool The mpool handle.
 *
 * \param ptr The chunk to query.
 *
 * \return The reference index, or -1 if does not exist.
 */
int mpool_ref_query(struct mpool * the_pool, const void* ptr);

/**
 * \brief Get the reference count of an allocated chunk in a \ref mpool.
 *
 * \note The returned value cannot be relied upon in concurrent contexts without
 * additional synchronization.
 *
 * \param the_pool The mpool handle.
 *
 * \param ptr The chunk to query.
 *
 * \return The reference count, or 0 on error.
 */
size_t mpool_ref_count(struct mpool * the_pool, const void* ptr);

END_C_DECLS
