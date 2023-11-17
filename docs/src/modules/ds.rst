.. _modules/ds:

===============
Data Structures
===============

A general, yet highly performant data structures library. It highly run-time
configurable, in the sense that the memory used by the data structure handle,
the elements the data structure will manage/contain, and structure metadata can
be independently provided by the calling application, or malloc()ed by the
library.

The data structures library is comprised of the following modules:

.. list-table::
   :header-rows: 1
   :widths: 20 70 10

   *  - Module

      - Notes

      - Link

   * - Ringbuffer

     -

     - :class:`rbuffer`

   * - Linked list.

     - Doubly linked.

     - :class:`llist`

   * - FIFO

     -  Built on ringbuffer.

     - :class:`fifo`

   * - Raw FIFO

     - Only handles {1, 2, 4} byte elements. Uses only pointer math when
       adding/removing elements, instead of memcpy()/function calls, so is ISR
       safe.

     - :class:`rawfifo`

   * - Dynamic array

     - Just like ``std::vector``,  grows/shrinks as needed. Uses approach
       in *Introduction To Algorithms*.

     - :class:`darray`

   * - Binary Search Tree

     - Uses approach in *Introduction To Algorithms*.

     - :class:`bstree`

   * - Red-Black tree

     - Uses approach in *Introduction To Algorithms*.

     - :class:`rbtree`

   * - Order Statistics Tree

     - Built on Red-Black Tree; uses approach in *Introduction To Algorithms*.

     - :class:`ostree`

   * - Interval Tree

     - Built on Red-Black Tree.  Uses approach in *Introduction To Algorithms*.

     - :class:`inttree`

   * - Hashmap

     - Built using dynamic arrays.

     - :class:`hashmap`

   * - Binary heap

     -  Built using dynamic array.

     - :class:`bin_heap`

   * - Matrix

     - Static matrix; dimensions cannot change after initialization.

     - :class:`matrix`

   * - Dynamic Matrix

     - Dimensions *can* change after initialization. Can be used to represent
       dynamic graphs. Works best on densely connected graphs.

     - :class:`dyn_matrix`

   * - Adjacency Matrix

     - Dimensions (# vertices) cannot change after initialization. Can be used
       to represent graphs efficiently; works best on densely connected
       graphs.

     - :class:`adj_matrix`

Common API
==========

All of the data structures (loosely) conform to the following API; not all data
structures implement all functions. E.g., :class:`llist` does not implement
``llist_data_get()`` because linked lists don't have a concept of indices.

.. list-table::
   :header-rows: 1
   :widths: 20 80

   * - Function

     - Purpose

   * - ``XX_init()``

     - Initialize the data structure. Any usage of the data structure handle
       prior to calling this function is undefined.

   * - ``XX_element_space()``

     - Give the max # of elements and the element size, compute the amount of
       space the calling application will need to reserve if it doesn't want the
       data structure to do any allocations for elements in the data structure.

   * - ``XX_meta_space()``

     - Give the max # of elements, compute the amount of
       space the calling application will need to reserve if it doesn't want the
       data structure to do any allocations for metadata to track the elements
       it contains. Only applicable to e.g., :class:`llist` and not
       :class:`darray`.

   * - ``XX_destroy()``

     - Destroy the data structure. Any usage of the data structure handle after
       calling this function is undefined until ``XX_init()`` is called again.


   * - ``XX_add()/XX_insert()``

     - Add a new element to the data structure.

   * - ``XX_remove()``

     - Remove an existing element from the data structure.

   * - ``XX_clear()``

     - Remove all elements from the data structure, but doesn't destroy it.

   * - ``XX_print()``

     - Print all elements of the data structure using the callback provided
       during initialization.


   * - ``XX_isfull()``

     - Is the data structure currently full ? i.e., any future attempts to try
       and add elements will fail unless one is removed first.

   * - ``XX_isempty()``

     - Is the data structure currently empty?

   * - ``XX_size()``

     - Get the current # of elements in the data structure.

   * - ``XX_capacity()``

     - Get the maximum # of elements that the data structure can currently
       handle. Only applies to data structures which have a fixed capacity at
       every point in time, such as an array. This is different that the maximum
       *possible* capacity for a data structure, which is set during
       initialization.

   * - ``XX_sort()``

     - Sort the data structure.

   * - ``XX_filter()/XX_filter2()``

     - Remove elements from the data structure instance into a new instance OR
       just delete them from the data structure according to a user-defined
       predicate.

   * - ``XX_copy()/XX_copy2()``

     - Make a copy of the data structure, possibly conditioning element
       membership in the new data structure via a user-defined predicate.

   * - ``XX_inject()``

     - Iterate over all elements in the data structure, computing a cumulative
       *something* using a user-defined predicate.

   * - ``XX_map()``

     - Apply a used-defined predicate to all elements in the data structure.

   * - ``XX_query()``

     - Query a data structure to see if a given element is present. Some data
       structures implement querying by key, some by key or index, and some
       implement multiple querying modalities.

   * - ``XX_data_get()``

     - Get an item from a data structure directly by index. No error checking is
       performed, so the index must be valid.
