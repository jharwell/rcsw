.. SPDX-License-Identifier: MIT

.. _library/swbus:

====================
Software Bus (SWBUS)
====================

SWBUS is a publish-subscribe message bus. Any number of threads or tasks can
publish arbitrarily-sized packets to named *packet IDs* (PIDs), and any number
of subscribers can receive those packets via independent receive queues
(RXQs). There is no centralized dispatcher: the publishing thread performs all
subscriber notification work inline during :c:func:`swbus_publish()`.

Architecture
============

Key concepts:

- **Packet ID (PID)** — A ``uint32_t`` identifying a message topic. Publishers
  and subscribers refer to the same PID to communicate.

- **Receive Queue (RXQ)** — A :c:struct:`pcqueue` (producer-consumer queue) owned
  by a subscriber. Created via :c:func:`swbus_rxq_init()` and then associated
  with one or more PIDs via :c:func:`swbus_subscribe()`.

- **Buffer Pool** — A :c:struct:`mpool` from which packet payload memory is
  allocated. One bus instance can have multiple pools of different element
  sizes; the bus uses the first pool, in configuration order, whose buffers
  are large enough for the packet and that has one free. List pools from
  smallest to largest to get the best fit. Memory is reference-counted: each subscribed RXQ holds
  one reference, which is released when the application calls
  :c:func:`swbus_rxq_pop_front()`.

- **Sync vs Async mode** — Controlled by the ``RCSW_SWBUS_ASYNC`` flag at
  init. In sync mode (default), a fair reader/writer lock
  (:c:struct:`rdwrlock`) ensures that subscriber threads cannot begin processing
  a packet until all subscribers to that PID have been notified. In async
  mode this guarantee is dropped; subscribers may observe the packet at
  different times.

Initialization
==============

::

   struct mpool_config pool_config = {
       .max_elts = 32,
       .elt_size = 128,   /* maximum packet size this pool can hold */
       /* .elements / .meta + RCSW_NOALLOC_* flags for caller storage */
   };

   struct swbus_config config = {
       .name      = "mybus",
       .max_rxqs  = 8,
       .max_subs  = 16,
       .max_pools = 1,
       .pools     = &pool_config,
       /* &bus below is caller storage; add RCSW_SWBUS_ASYNC to opt out of
        * sync guarantees */
       .flags     = RCSW_NOALLOC_HANDLE,
   };

   struct swbus bus;
   if (NULL == swbus_init(&bus, &config)) {
       /* initialization failed */
   }

   /* Create a receive queue (8-entry depth, library-allocated storage) */
   struct pcqueue* rxq = swbus_rxq_init(&bus, NULL, 8);

   /* Subscribe the RXQ to PID 0x10 */
   swbus_subscribe(&bus, rxq, 0x10);

Publishing
==========

Simple publish (one call)
--------------------------

::

   uint8_t pkt[64] = { /* ... */ };
   swbus_publish(&bus, 0x10, sizeof(pkt), pkt);

Two-phase publish (reserve then release)
-----------------------------------------

For zero-copy or scatter-gather scenarios, you can write directly into the
bus-allocated buffer before committing. The packet size subscribers see is
``res.pkt_size``; set it lower before releasing to publish less than you
reserved::

   struct swbus_rsrvn res;
   if (OK == swbus_publish_reserve(&bus, &res, sizeof(pkt))) {
       memcpy(res.data, pkt, sizeof(pkt));
       swbus_publish_release(&bus, 0x10, &res);
   }

.. NOTE::

   If :c:func:`swbus_publish_reserve()` succeeds but
   :c:func:`swbus_publish_release()` is never called, the reserved buffer
   will leak until the pool is destroyed. Always pair reserve with release.

A reservation can also be built by hand, with ``data`` pointing at a buffer
the application owns, ``pkt_size`` set, and ``bp = NULL``. Nothing is copied
and no pool is involved, so the application must keep the buffer valid until
every subscriber has called :c:func:`swbus_rxq_pop_front()`.

Receiving
=========

::

   /* Block until a packet arrives on this RXQ */
   struct swbus_rxq_ent* ent = swbus_rxq_wait(&bus, rxq);
   if (ent != NULL) {
       /* Access packet: ent->data, ent->pkt_size, ent->pid */
       process(ent->data, ent->pkt_size);

       /* Release buffer reference and pop from queue */
       swbus_rxq_pop_front(rxq, ent);
   }

   /* Front entry, without waiting: NULL (errno = EAGAIN) if the queue is empty */
   struct swbus_rxq_ent* front = swbus_rxq_front(rxq);

   /* Timed wait (relative timeout; see the concurrency concepts page) */
   struct timespec timeout = { .tv_sec = 1, .tv_nsec = 0 };
   ent = swbus_rxq_timedwait(&bus, rxq, &timeout);

.. WARNING::

   Always call :c:func:`swbus_rxq_pop_front()` when done with an entry.
   Failing to do so leaks the buffer-pool reference; if all references to a
   pool chunk are not released the pool will eventually exhaust.

API Summary
===========

.. list-table::
   :header-rows: 1
   :widths: 35 65

   * - Function
     - Purpose

   * - :c:func:`swbus_init()`
     - Initialize a bus instance. Returns ``NULL`` on failure.

   * - :c:func:`swbus_destroy()`
     - Destroy the bus and release all internally allocated memory.

   * - :c:func:`swbus_rxq_init()`
     - Allocate and initialize a new receive queue. Returns a
       :c:struct:`pcqueue` pointer, or ``NULL`` if the maximum number of RXQs
       has been reached.

   * - :c:func:`swbus_subscribe()`
     - Associate an RXQ with a PID. Returns ``ERROR`` if the subscription
       already exists or the subscription list is full.

   * - :c:func:`swbus_unsubscribe()`
     - Dissociate an RXQ from a PID. Returns ``ERROR`` if no such
       subscription exists.

   * - :c:func:`swbus_publish()`
     - Convenience wrapper: reserve a buffer, copy the caller's packet into
       it, and notify all subscribers. Returns ``ERROR`` if no pool has a
       free buffer large enough for ``pkt_size`` (``ENOSPC``), or if a
       subscribed RXQ is full; publishing never blocks on a full RXQ.

   * - :c:func:`swbus_publish_reserve()`
     - Allocate a buffer from the appropriate pool and return a reservation
       handle. Does not notify subscribers.

   * - :c:func:`swbus_publish_release()`
     - Notify all subscribers of a PID using a previously reserved buffer.
       Releases the pool reference if no subscribers are registered for the
       PID.

   * - :c:func:`swbus_rxq_wait()`
     - Block until a packet is available in the RXQ. Returns a pointer to
       the front entry, or ``NULL`` on error.

   * - :c:func:`swbus_rxq_timedwait()`
     - Like :c:func:`swbus_rxq_wait()` but returns ``NULL`` after the
       specified timeout if no packet arrives.

   * - :c:func:`swbus_rxq_front()`
     - Return the front entry without waiting, or ``NULL`` if the RXQ is
       empty. Takes no bus handle, so in sync mode it doesn't wait for an
       in-progress publish to reach every subscriber.

   * - :c:func:`swbus_rxq_pop_front()`
     - Release the buffer-pool reference for the front entry and remove it
       from the queue. Must be called after the application is done
       processing a received entry.

Thread Safety
=============

See also :ref:`concepts/concurrency`.

Creating RXQs, subscribing, unsubscribing and releasing a publish all take
the bus's internal :c:struct:`mutex`, so they can be called from any thread
at any time; they are serialized with each other. In sync mode a
:c:struct:`rdwrlock` additionally keeps receivers in :c:func:`swbus_rxq_wait()`
/ :c:func:`swbus_rxq_timedwait()` from seeing a packet until it has reached
every subscriber. RXQs are :c:struct:`pcqueue` instances and are thread-safe
on their own. Don't call any function on a bus after
:c:func:`swbus_destroy()`.
