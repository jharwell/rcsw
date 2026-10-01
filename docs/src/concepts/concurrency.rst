.. SPDX-License-Identifier: MIT

.. _concepts/concurrency:

===========
Concurrency
===========

Most of RCSW does no locking of its own. The pieces that do are built for it
and say so; everything else needs the caller to serialize access.

What is thread-safe
===================

.. list-table::
   :header-rows: 1
   :widths: 35 65

   * - Component
     - Guarantee

   * - :ref:`library/multithread` primitives, :c:struct:`mpool`,
       :c:struct:`pcqueue`
     - Safe to call from any thread.

   * - :ref:`library/swbus`
     - Safe to call from any thread: publishing, receiving, creating receive
       queues and changing subscriptions are all serialized internally.

   * - :c:struct:`rawfifo`
     - Lock-free for exactly one producer and one consumer, such as an ISR and
       the main loop. More of either needs a lock.

   * - :c:struct:`multififo`
     - Add and remove take an internal busy flag; an asynchronous consumer
       checks :c:func:`multififo_islocked` and backs off. Calls that find the
       flag held fail with ``EAGAIN``. Everything else must be called from one
       context.

   * - LOG4CL plugin
     - Module installation, removal and lookup are serialized internally on
       POSIX.

   * - All other data structures
     - Not thread-safe. Lock around every call.

The multithread primitives and the timespec helpers in ``rcsw/utils/time.h``
need POSIX and are not available in bare-metal builds.

Snapshot values
===============

Size and state queries on thread-safe structures (``mpool_size()``,
``pcqueue_size()``, ``pcqueue_n_free()``, ``rawfifo_size()``, peeks, ...)
return the value at the moment of the call. Another thread can change it
before the caller acts on it, so use them for monitoring, not to decide
whether a following call will succeed. Capacities don't change and are safe to
rely on.

.. _concepts/concurrency/timeouts:

Timeouts
========

Blocking calls with a timeout take a **relative** timeout: how long to wait
from now. This differs from the POSIX functions they wrap, which take an
absolute time. RCSW converts it against ``CLOCK_REALTIME`` with
:c:func:`utils_ts_make_abs`.

A relative timeout restarts on every call. Code that waits in a loop, for
example to handle spurious wake-ups, would never time out. For that case use
the ``_abs`` variants (:c:func:`condv_timedwait_abs`,
:c:func:`csem_timedwait_abs`), which take an absolute ``CLOCK_REALTIME``
deadline. :c:func:`utils_ts_make_abs` builds the deadline once before the
loop, and :c:func:`utils_ts_make_rel` turns it back into the time remaining.

.. code-block:: c

   struct timespec rel = {.tv_sec = 1, .tv_nsec = 0};
   struct timespec deadline;
   utils_ts_make_abs(&rel, &deadline);

   mutex_lock(&mtx);
   while (!ready) {
     if (OK != condv_timedwait_abs(&cv, &mtx, &deadline)) {
       break; /* errno == ETIMEDOUT once the deadline passes */
     }
   }
   mutex_unlock(&mtx);
