.. SPDX-License-Identifier: MIT

.. _library/ds:

===============
Data Structures
===============

A general, yet highly performant data structures library. It is highly
run-time configurable: the memory used by the data structure handle, the
elements the data structure will manage, and structure metadata can each be
independently provided by the calling application or ``malloc()``\ed by the
library.

.. WARNING::

   **Data structures in this module are not thread-safe.** No internal
   synchronization is performed. Callers are responsible for all external
   locking. See :ref:`library/multithread` for suitable primitives
   (e.g., :c:struct:`mutex`, :c:struct:`rdwrlock`).

Available Structures
====================

.. list-table::
   :header-rows: 1
   :widths: 20 60 10 10

   * - Module
     - Notes
     - Link
     - Time Complexity (typical ops)

   * - Ringbuffer
     - Fixed-capacity circular buffer. Overwrite mode (oldest element
       evicted) or FIFO mode (add fails when full) selectable at init via
       :c:macro:`RCSW_DS_RBUFFER_AS_FIFO`.
     - :c:struct:`rbuffer`
     - Add/remove: O(1)

   * - Linked list
     - Doubly linked. Supports optional sorted-insertion mode
       (:c:macro:`RCSW_DS_SORTED`). Uncapped length supported unless
       :c:macro:`RCSW_NOALLOC_META` or :c:macro:`RCSW_NOALLOC_DATA` is set, in
       which case ``max_elts`` must be finite.
     - :c:struct:`llist`
     - Append/prepend: O(1); query: O(n); sort: O(n log n)

   * - FIFO
     - Built on :c:struct:`rbuffer`. Strict FIFO semantics; add fails when
       full.
     - :c:struct:`fifo`
     - Push/pop: O(1)

   * - Multi-FIFO
     - A FIFO of large elements, each consumed in smaller pieces through up
       to 8 child FIFOs that point into it. Data is never copied to the
       children.
     - :c:struct:`multififo`
     - Push/pop: see API reference

   * - Raw FIFO
     - Only handles {1, 2, 4}-byte elements. Lock-free for exactly one
       producer and one consumer, so it can pass data between an ISR and the
       main loop.
     - :c:struct:`rawfifo`
     - Push/pop: O(1)

   * - Dynamic array
     - Analogous to ``std::vector``; grows/shrinks as needed using the
       approach from *Introduction To Algorithms*.
     - :c:struct:`darray`
     - Append: amortized O(1); index: O(1); search: O(n)

   * - Binary Search Tree
     - Uses approach in *Introduction To Algorithms*.
     - :c:struct:`bstree`
     - Insert/remove/query: O(h), worst case O(n) on unbalanced tree

   * - Red-Black Tree
     - Self-balancing BST: a :c:struct:`bstree` created with
       :c:macro:`RCSW_DS_BSTREE_RB`. Uses approach in *Introduction To
       Algorithms*.
     - :c:struct:`bstree`
     - Insert/remove/query: O(log n) guaranteed

   * - Order Statistics Tree
     - Built on Red-Black Tree. Supports order-statistic queries (rank,
       select). Uses approach in *Introduction To Algorithms*.
     - :c:func:`ostree_init`
     - All BST ops + rank/select: O(log n)

   * - Interval Tree
     - Built on Red-Black Tree. Finds a stored interval that overlaps a given
       one. Uses approach in *Introduction To Algorithms*.
     - :c:func:`inttree_init`
     - Insert/query: O(log n)

   * - Hashmap
     - Fixed-size buckets, each a dynamic array. Keys are
       :c:macro:`RCSW_HASHMAP_KEYSIZE` (64) bytes. When a bucket is full, an
       add fails unless :c:macro:`RCSW_DS_HASHMAP_LINPROB` is set, which
       probes the following buckets. The hash function is supplied in
       :c:member:`hashmap_config.hash`; ``rcsw/utils/hash.h`` provides
       several.
     - :c:struct:`hashmap`
     - Insert/query: amortized O(1); worst case O(n) under pathological
       hash collisions

   * - Binary heap
     - Built using dynamic array. A max heap, or a min heap with
       :c:macro:`RCSW_DS_BINHEAP_MIN`.
     - :c:struct:`binheap`
     - Insert: O(log n); peek-min/max: O(1); extract-min/max: O(log n)

   * - Matrix
     - Static matrix; dimensions cannot change after initialization.
     - :c:struct:`matrix`
     - Element access: O(1)

   * - Dynamic Matrix
     - Dimensions *can* change after initialization. Can be used to
       represent dynamic graphs. Works best on densely connected graphs.
     - :c:struct:`dynmatrix`
     - Element access: O(1); resize: O(m*n)

   * - Adjacency Matrix
     - Dimensions (# vertices) cannot change after initialization.
       Efficient graph representation; works best on densely connected
       graphs.
     - :c:struct:`adjmatrix`
     - Edge query: O(1)

Common API
==========

Conventions shared by every structure (configuration, callbacks, element
ownership, iterators, flags) are described in
:ref:`concepts/data-structures`.

All data structures (loosely) conform to the following API. Not all
structures implement every function — e.g., :c:struct:`llist` does not
implement ``llist_data_get()`` because linked lists do not have an index
concept.

.. list-table::
   :header-rows: 1
   :widths: 22 78

   * - Function
     - Purpose

   * - ``XX_init()``
     - Initialize the data structure. Usage of the handle prior to calling
       this function is undefined behavior. Returns ``NULL`` on failure;
       ``errno`` is set (see :ref:`library/ds/error-codes`).

   * - ``XX_element_space()``
     - Given the max number of elements and the element size, returns the
       number of bytes the caller must reserve if it wants to supply
       element storage and pass :c:macro:`RCSW_NOALLOC_DATA`.

   * - ``XX_meta_space()``
     - Given the max number of elements, returns the number of bytes the
       caller must reserve if it wants to supply metadata storage and pass
       :c:macro:`RCSW_NOALLOC_META`. Only applicable to structures with
       per-element metadata (e.g., :c:struct:`llist`), not contiguous-array
       structures (e.g., :c:struct:`darray`).

   * - ``XX_destroy()``
     - Destroy the data structure and release any internally allocated
       memory. Usage of the handle after calling this function is
       undefined until ``XX_init()`` is called again.

   * - ``XX_add()`` / ``XX_insert()``
     - Add a new element. Returns ``ERROR`` with ``errno = ENOSPC`` if
       the structure is full. See :ref:`library/ds/error-codes`.

   * - ``XX_remove()``
     - Remove an element: by value (:c:struct:`llist`, :c:struct:`hashmap`,
       :c:struct:`bstree`), by index (:c:struct:`darray`) or from the front
       (:c:struct:`fifo`, :c:struct:`rbuffer`). Removing an element that
       isn't present returns ``ERROR`` with ``errno = ENOENT``.

   * - ``XX_clear()``
     - Remove all elements without destroying the structure. The handle
       remains valid and ``XX_init()`` does not need to be called again.

   * - ``XX_print()``
     - Print all elements using the ``printe`` callback provided during
       initialization. Requires that callback.

   * - ``XX_isfull()``
     - Returns ``true`` if no further elements can be added without first
       removing one.

   * - ``XX_isempty()``
     - Returns ``true`` if the structure contains no elements.

   * - ``XX_size()``
     - Returns the current number of elements.

   * - ``XX_capacity()``
     - Returns the maximum number of elements the structure can currently
       hold. Only defined for structures with a fixed capacity at any
       point in time (e.g., :c:struct:`rbuffer`). Distinct from the *maximum
       possible* capacity set during initialization.

   * - ``XX_sort()``
     - Sort the structure in place using the comparator provided at init.

   * - ``XX_filter()`` / ``XX_remove_if()``
     - Remove the elements matching a caller-supplied predicate.
       ``XX_filter()`` moves them into a new instance; ``XX_remove_if()``
       deletes them in place.

   * - ``XX_copy()`` / ``XX_copy_if()``
     - Copy the structure; ``XX_copy_if()`` copies only the elements matching
       a caller-supplied predicate.

   * - ``XX_inject()``
     - Iterate over all elements, computing a cumulative result via a
       caller-supplied accumulator function (analogous to a fold/reduce).

   * - ``XX_map()``
     - Apply a caller-supplied function to every element in place.

   * - ``XX_*_query()``
     - Find an element. Depending on the structure this returns the element
       (``llist_data_query()``), its node (``llist_node_query()``) or its
       index (``darray_idx_query()``). See per-structure documentation.

   * - ``XX_data_get()``
     - Return a pointer to an element by index. Out-of-range indices are
       the caller's error, and the response varies: :c:func:`rbuffer_data_get`
       returns ``NULL``, while :c:func:`darray_data_get` reports a FATAL event
       and fails an ``assert()``.

.. _library/ds/memory:

Memory
======

Each structure can take its handle, element storage and metadata from the
caller instead of the heap. See :ref:`concepts/memory-model` for the flags,
sizing rules, zeroing and alignment, and :ref:`library/ds/quickstart` for an
example.

.. _library/ds/error-codes:

Error Codes
===========

On failure, functions return ``NULL`` (pointer-returning) or ``ERROR``
(``status_t``-returning) and, on most paths, set ``errno``:

.. list-table::
   :header-rows: 1
   :widths: 15 85

   * - ``errno``
     - Meaning

   * - ``EINVAL``
     - Invalid argument. Set by every function's precondition checks (see
       :ref:`concepts/error-handling/fpc`) and by a few explicit checks in
       :c:struct:`darray`, :c:struct:`llist` and :c:struct:`multififo`.

   * - ``ENOMEM``
     - An allocation failed (:c:struct:`darray`, :c:struct:`llist`,
       :c:struct:`hashmap`, :c:struct:`binheap`, :c:struct:`multififo`).

   * - ``ENOSPC``
     - The structure is full (:c:struct:`darray`, :c:struct:`llist`,
       :c:struct:`hashmap`, :c:struct:`binheap`, :c:struct:`bstree`, and
       :c:struct:`rbuffer` in FIFO mode).

   * - ``EEXIST``
     - The key is already present (:c:struct:`hashmap`, :c:struct:`bstree`).

   * - ``ENOENT``
     - The element, key or edge isn't present (:c:func:`llist_remove`,
       :c:func:`llist_splice`, :c:func:`hashmap_remove`,
       :c:func:`adjmatrix_edge_remove`).

   * - ``EAGAIN``
     - :c:func:`bstree_init` or :c:func:`fifo_init` failed partway through,
       or :c:struct:`multififo` is busy (see :c:func:`multififo_islocked`).

.. NOTE::

   Not every error path in every module sets ``errno``. If precise error
   discrimination is needed, rely on the ``ERROR`` return value for
   control flow and treat ``errno`` as advisory. This inconsistency is a
   known limitation and is tracked for improvement.

.. _library/ds/quickstart:

Quickstart Example
==================

The following creates a :c:struct:`llist` that uses no heap: the handle,
element storage and node storage all come from the caller. The sizing
functions are not constant expressions, so the static buffers are sized
generously and checked before use.

.. code-block:: c

   #include "rcsw/ds/llist.h"

   #define N_ELTS 16

   static dptr_t elements[512 / sizeof(dptr_t)];
   static dptr_t nodes[1024 / sizeof(dptr_t)];
   static struct llist list;

   status_t list_setup(void) {
     if (sizeof(elements) < llist_element_space(N_ELTS, sizeof(int)) ||
         sizeof(nodes) < llist_meta_space(N_ELTS)) {
       return ERROR;
     }
     struct llist_config config = {
       .cmpe     = NULL,
       .printe   = NULL,
       .elements = elements,
       .meta     = nodes,
       .elt_size = sizeof(int),
       .max_elts = N_ELTS,
       .flags    = RCSW_NOALLOC_ALL,
     };
     if (NULL == llist_init(&list, &config)) {
       return ERROR; /* errno says why */
     }

     int val = 42;
     llist_append(&list, &val); /* copies val into the list */

     /* ... use the list ... */

     llist_destroy(&list); /* frees nothing: all memory is the caller's */
     return OK;
   }

See the test suite for extensive usage examples covering all data
structures.
