.. _modules-multithread:

Multithread
===========

A collection of modules for programming in multithreaded environments (obviously
all thread safe).

.. NOTE:: Provides some wrappers around POSIX multithread primitives (e.g.,
          sem_t) which may seem unncessary, BUT provides a small Platform
          Abstraction Layer (PAL) so that applications can use the same API on
          Linux, RTEMS, etc.

The multithread library is comprised of the following modules:

.. list-table::
   :header-rows: 1
   :widths: 20 70 10

   *  - Module
      - Notes
      - Link

   * - Memory pool
     - Used by threads to request/release memory chunks of a specified
       size. Useful in publisher-subscriber settings (e.g., :class:`pulse`).

     - :class:`mpool`

   * - Binary semaphore
     - On linux: built out of a :class:`mutex` and :class:`condv`.
     - :class:`bsem`

   * - Condition variable
     - On linux: Wrapper around POSIX condition variables.
     - :class:`condv`

   * - Counting semaphore
     - On linux: wrapper around POSIX semaphores.
     - :class:`csem`

   * - Mutex
     - On linux: wrapper around POSIX mutexes.
     - :class:`mutex`

   * - Condition variable/mutex pair (cvm)
     - A single interface for using both, as they are frequently used together.
     - :class:`cvm`

   * - Producer-consumer queue
     - Can handle multiple producers and consumers
     - :class:`pcqueue`

   * - Fair reader/writer lock
     - A completely fair lock that guarantees that neither readers nor writers
       will starve.
     - :class:`rdwrlock`

   * - OpenMP modules
     - Implementations of 2D kernel convolution, radix sort
     - :c:func:`omp_kernel2d_convolve1`, :class:`omp_radix_sorter`

   * - Thread management tools
     - E.g., locking threads to a particular core.
     - N/A
