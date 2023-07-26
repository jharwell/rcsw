.. _ln-rcsw-ds:

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
       to represent graphs efficiently; Works best on densely connected
       graphs.
     - :class:`adj_matrix`
