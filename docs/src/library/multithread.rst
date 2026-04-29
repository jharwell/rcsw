.. _library/multithread:

===========
Multithread
===========

A collection of modules for multithreaded programming. All primitives in this
module are thread-safe by design. Where applicable, POSIX primitives are
wrapped to provide a small Platform Abstraction Layer (PAL) so that
application code compiles unmodified on Linux, RTEMS, and similar targets.

.. NOTE::

   All primitives are initialized to use caller-supplied storage when
   ``RCSW_NOALLOC_HANDLE`` is passed; pass ``NULL`` as the handle argument
   to have the library allocate.

Primitives
==========

.. list-table::
   :header-rows: 1
   :widths: 25 60 15

   * - Module
     - Notes
     - Link

   * - Mutex
     - Wrapper around POSIX mutexes (``pthread_mutex_t``). Non-recursive.
     - :c:struct:`mutex`

   * - Condition variable
     - Wrapper around POSIX condition variables (``pthread_cond_t``).
     - :c:struct:`condv`

   * - Condition variable / mutex pair
     - Convenience wrapper combining :c:struct:`condv` and :c:struct:`mutex` into
       a single handle, since they are almost always used together.
     - :c:struct:`cvm`

   * - Binary semaphore
     - Built on :c:struct:`mutex` + :c:struct:`condv` on Linux. Provides
       signal/wait semantics with an initial value of 0 or 1.
     - :c:struct:`bsem`

   * - Counting semaphore
     - Wrapper around POSIX semaphores (``sem_t``).
     - :c:struct:`csem`

   * - Fair reader/writer lock
     - Guarantees that neither readers nor writers will starve. Uses three
       :c:struct:`csem` instances (``order``, ``access``, ``read``) to enforce
       strict arrival-order fairness. Suitable for protecting shared data
       structures (e.g., ds/ modules) from concurrent access.
     - :c:struct:`rdwrlock`

Higher-Level Constructs
=======================

.. list-table::
   :header-rows: 1
   :widths: 25 60 15

   * - Module
     - Notes
     - Link

   * - Memory pool
     - Thread-safe fixed-size block allocator. Used by threads to
       request/release memory chunks of a specified size. Reference-counted;
       a block is returned to the pool only when all holders have released
       it. Particularly useful in publisher-subscriber settings
       (e.g., :c:struct:`swbus`).
     - :c:struct:`mpool`

   * - Producer-consumer queue
     - Blocking FIFO queue supporting multiple producers and consumers.
       Internally synchronized; callers do not need additional locking.
     - :c:struct:`pcqueue`

Parallel Algorithms
===================

.. list-table::
   :header-rows: 1
   :widths: 30 55 15

   * - Module
     - Notes
     - Link

   * - OpenMP 2D kernel convolution
     - Parallelizes 2D convolution of an image/signal with a kernel using
       OpenMP. Requires an OpenMP-capable compiler.
     - :c:func:`omp_kernel2d_convolve1`

   * - OpenMP radix sort
     - Parallel radix sort using OpenMP. Requires an OpenMP-capable
       compiler.
     - :c:struct:`omp_radix_sorter`

Thread Management
=================

Utilities for thread lifecycle management, including pinning threads to
specific CPU cores (affinity). These utilities are POSIX-only and are not
available in baremetal builds.
