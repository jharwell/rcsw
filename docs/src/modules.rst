.. _ln-rcsw-modules:

===============
Modules Summary
===============

RCSW is built around the idea of modules to help organize the code, and to
provide logical segmentation between (un)complementary areas of
functionality. Full details of modules is in the documentation, which can be
built with `make apidoc`. Only a brief summary is given here.

Most modules come with at least a cursory test suite to provide sanity checks,
though many have thorough suites. Test suites also serve as example usages.

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

Algorithms
==========

A general purpose set of *serial* algorithms. Includes:

- String parenthesization: Can a string be parenthesized to result in another
  string under the rules of some multiplicative paradigm?
- Edit Distance: How close are two contiguous sequences of characters, numbers,
  structs, etc. to each other, according to some measure?
- Longest Common Subsequence: What is the longest common subsequence of two
  sequences of objects?
- Matrix Chain Optimization: What is the most efficient way to multiply a
  sequence of matrices?
- Binary Search (recursive and iterative).
- Quicksort (recursive and iterative).
- Mergesort (recursive and iterative).
- Insertion sort.
- Radix sort.

Multithread
===========

A collection of modules for programming in multithreaded environments (obviously
all thread safe). Full documentation is in. The wrappers around POSIX primitives
may seem uncessary, but I wanted to leave the ability to program on embedded
platforms where POSIX might not be available, and retain the ability to use a
single extensible interface.

- Memory pool: Used by threads to request/release memory chunks of a specified
  size. Useful in publisher-subscriber settings.
- Binary semaphore: Linux doesn't have one of these natively, so I built one out
  of a mutex and cv.
- Condition variable: Wrapper around POSIX condition variables.
- Counting semaphore: Wrapper around POSIX semaphores.
- mutex: Wrapper around POSIX mutexes.
- Condition variable/mutex pair (cvm): A single interface for using both, as
  they are frequently used together.
- Queue: Producer-consumer queue.
- Reader/Writer lock: A completely fair lock that guarantees that neither
  readers nor writers will starve.
- OpenMP implementations of 2D kernel convolution, radix sort.
- Various thread management tools like locking threads to a particular core.

Multiprocess
============

A collection of modules for managing multi-process applications, as well as a
few simple-ish MPI routines.

Simple I/O
==========

A re-implementation of the libc routines needed to make printf() work

I originally did this as a programming exercise, but have found it very useful
for debugging in bare metal/embedded environments when libc is not available
(e.g., bootstraps).

Utils
=====

A miscellaneous collection of functions and macros that I found useful at one
time or another, but that did not warrant their own module. Full documentation
is in utils. Includes:

- 8, 16 32 bit additive and XOR checksums
- Common hashing functions
- Memory manipulation (e.g., read-modify-write with verify)
- Timespec comparison/addition/etc.
- Random other things :-P

PUbLisher SubscribEr (PULSE)
============================

A publisher subscriber system, in which any number of
tasks/threads/processes/etc. can send arbitrarily sized packets to each other
using a distributed FIFO system. There is no centralized controller, meaning
each publishing thread does the work of its publish().
