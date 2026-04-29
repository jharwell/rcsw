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

   * - Raw FIFO
     - Only handles {1, 2, 4}-byte elements. Uses only pointer arithmetic
       when adding/removing elements—no ``memcpy()`` or function calls—so
       it is ISR-safe.
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
     - Self-balancing BST. Uses approach in *Introduction To Algorithms*.
     - :c:struct:`rbtree`
     - Insert/remove/query: O(log n) guaranteed

   * - Order Statistics Tree
     - Built on Red-Black Tree. Supports order-statistic queries (rank,
       select). Uses approach in *Introduction To Algorithms*.
     - :c:struct:`ostree`
     - All BST ops + rank/select: O(log n)

   * - Interval Tree
     - Built on Red-Black Tree. Uses approach in *Introduction To
       Algorithms*.
     - :c:struct:`inttree`
     - Insert/query: O(log n)

   * - Hashmap
     - Built using dynamic arrays. Open addressing with linear probing.
       Uses FNV-1a as the default hash.
     - :c:struct:`hashmap`
     - Insert/query: amortized O(1); worst case O(n) under pathological
       hash collisions

   * - Binary heap
     - Built using dynamic array. Min or max heap depending on the
       comparator provided at init.
     - :c:struct:`bin_heap`
     - Insert: O(log n); peek-min/max: O(1); extract-min/max: O(log n)

   * - Matrix
     - Static matrix; dimensions cannot change after initialization.
     - :c:struct:`matrix`
     - Element access: O(1)

   * - Dynamic Matrix
     - Dimensions *can* change after initialization. Can be used to
       represent dynamic graphs. Works best on densely connected graphs.
     - :c:struct:`dyn_matrix`
     - Element access: O(1); resize: O(m*n)

   * - Adjacency Matrix
     - Dimensions (# vertices) cannot change after initialization.
       Efficient graph representation; works best on densely connected
       graphs.
     - :c:struct:`adj_matrix`
     - Edge query: O(1)

Common API
==========

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
       ``errno`` is set (see :ref:`ds-error-codes`).

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
       the structure is full. See :ref:`ds-error-codes`.

   * - ``XX_remove()``
     - Remove an existing element. **Semantics vary by structure:** most
       structures return ``ERROR`` if the element is not present, but
       :c:struct:`llist` treats removal of a non-existent element as a no-op
       and returns ``OK``. Consult per-structure documentation.

   * - ``XX_clear()``
     - Remove all elements without destroying the structure. The handle
       remains valid and ``XX_init()`` does not need to be called again.

   * - ``XX_print()``
     - Print all elements using the callback provided during
       initialization. No-op if no print callback was provided.

   * - ``XX_isfull()``
     - Returns non-zero if no further elements can be added without first
       removing one.

   * - ``XX_isempty()``
     - Returns non-zero if the structure contains no elements.

   * - ``XX_size()``
     - Returns the current number of elements.

   * - ``XX_capacity()``
     - Returns the maximum number of elements the structure can currently
       hold. Only defined for structures with a fixed capacity at any
       point in time (e.g., :c:struct:`rbuffer`). Distinct from the *maximum
       possible* capacity set during initialization.

   * - ``XX_sort()``
     - Sort the structure in place using the comparator provided at init.

   * - ``XX_filter()`` / ``XX_filter2()``
     - Remove elements from the structure according to a caller-supplied
       predicate. ``XX_filter()`` moves matching elements into a new
       instance; ``XX_filter2()`` deletes them in place.

   * - ``XX_copy()`` / ``XX_copy2()``
     - Copy the structure, optionally filtering element membership in the
       new instance via a caller-supplied predicate.

   * - ``XX_inject()``
     - Iterate over all elements, computing a cumulative result via a
       caller-supplied accumulator function (analogous to a fold/reduce).

   * - ``XX_map()``
     - Apply a caller-supplied function to every element in place.

   * - ``XX_query()``
     - Test whether a given element is present. Some structures support
       query by key only; others support query by key or index, or
       multiple modalities. See per-structure documentation.

   * - ``XX_data_get()``
     - Return a pointer to an element by index. **No bounds checking is
       performed;** the caller must ensure the index is valid.

.. _ds-memory-model:

Memory Model
============

Each data structure independently controls three memory regions:

.. list-table::
   :header-rows: 1
   :widths: 20 40 40

   * - Region
     - Flag (``RCSW_NOALLOC_*``)
     - Notes

   * - Handle
     - :c:macro:`RCSW_NOALLOC_HANDLE`
     - The ``struct XX`` itself. Pass a caller-allocated handle as the
       first argument to ``XX_init()``; ``NULL`` causes the library to
       ``malloc()`` one.

   * - Data
     - :c:macro:`RCSW_NOALLOC_DATA`
     - Storage for element payloads. Pass via ``params->elements``;
       ``NULL`` causes the library to ``malloc()`` the required space
       (use ``XX_element_space()`` to compute it).

   * - Metadata
     - :c:macro:`RCSW_NOALLOC_META`
     - Per-element bookkeeping (e.g., :c:struct:`llist` node structs). Pass
       via ``params->meta``; ``NULL`` causes the library to ``malloc()``
       it. **Cannot be used with uncapped (max_elts = -1) data structures.**

Setting :cmake:variable:`RCSW_CONFIG_NOALLOC` at build time implies all three flags for
all modules.

.. _ds-error-codes:

Error Codes
===========

On failure, functions return ``NULL`` (pointer-returning) or ``ERROR``
(``status_t``-returning) and set ``errno`` as follows:

.. list-table::
   :header-rows: 1
   :widths: 15 20 65

   * - ``errno``
     - Modules
     - Meaning

   * - ``ENOMEM``
     - :c:struct:`llist`, :c:struct:`llist_node`
     - Internal node or data allocation failed.

   * - ``ENOSPC``
     - :c:struct:`llist`, :c:struct:`rbuffer`, :c:struct:`darray`
     - Structure is full; element cannot be added.

   * - ``EINVAL``
     - :c:struct:`llist`
     - Invalid argument (e.g., attempting to remove from an empty list,
       or splice with invalid indices).

   * - ``EAGAIN``
     - :c:struct:`darray`, :c:struct:`hashmap`, :c:struct:`bstree`,
       :c:struct:`bin_heap`, :c:struct:`fifo`, :c:struct:`multififo`
     - Internal resource temporarily unavailable (e.g., allocation
       failure during resize or node creation).

.. NOTE::

   Not every error path in every module sets ``errno``. If precise error
   discrimination is needed, rely on the ``ERROR`` return value for
   control flow and treat ``errno`` as advisory. This inconsistency is a
   known limitation and is tracked for improvement.

Quickstart Example
==================

The following shows a minimal usage of :c:struct:`llist` with caller-managed
memory (no heap allocation).

.. NOTE::

   The sizing functions ``llist_element_space()`` and
   ``llist_meta_space()`` return values that are computed at compile time
   from constant arguments, making them suitable as array sizes in C99.
   In C++ or with ``-Wvla``, use ``static`` arrays sized conservatively,
   or allocate with ``malloc`` and pass ``RCSW_NOALLOC_DATA`` /
   ``RCSW_NOALLOC_META`` only when you need strict no-heap guarantees.

.. code-block:: c

   #include "rcsw/ds/llist.h"

   /* Stack-allocate storage for up to 16 int-sized elements */
   static uint8_t elts[llist_element_space(16, sizeof(int))];
   static uint8_t meta[llist_meta_space(16)];

   struct llist list;
   struct llist_params p = {
       .max_elts  = 16,
       .elt_size  = sizeof(int),
       .elements  = elts,
       .meta      = meta,
       .flags     = RCSW_NOALLOC_HANDLE | RCSW_NOALLOC_DATA | RCSW_NOALLOC_META,
   };

   if (NULL == llist_init(&list, &p)) {
       /* errno is set; inspect it for details */
       return ERROR;
   }

   int val = 42;
   llist_append(&list, &val);

   /* ... use the list ... */

   llist_destroy(&list);

See the test suite for extensive usage examples covering all data
structures.
