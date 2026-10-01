.. SPDX-License-Identifier: MIT

.. _library/multithread:

===========
Multithread
===========

Thin wrappers around the POSIX threading primitives, plus a memory pool and a
producer-consumer queue built on them. Every module here is thread-safe by
design, and POSIX-only: none of it is available in bare-metal builds.

.. NOTE::

   To use caller-supplied storage for a primitive's handle, pass it with
   :c:macro:`RCSW_NOALLOC_HANDLE`. Without that flag the handle argument is
   ignored (``NULL`` is fine) and the library allocates one. See
   :ref:`concepts/memory-model`.

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
       a single handle, since they are almost always used together. As with a
       plain condition variable, the caller holds the mutex
       (:c:member:`cvm.mtx`) around waits.
     - :c:struct:`cvm`

   * - Binary semaphore
     - Built on :c:struct:`mutex` + :c:struct:`condv`. Provides post, wait,
       timed wait, and flush (release every waiter).
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

Thread Management
=================

``#include "rcsw/multithread/threadm.h"``

:c:func:`threadm_core_lock` pins a thread to a CPU core. Like the rest of this
module it is POSIX-only; it uses ``pthread_setaffinity_np()``, so it needs
Linux or another platform that provides it.

For timeouts and what is safe to call concurrently, see
:ref:`concepts/concurrency`.
