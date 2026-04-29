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
  sizes; the bus selects the smallest pool whose buffers are large enough to
  hold a given packet. Memory is reference-counted: each subscribed RXQ holds
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

   struct mpool_params pool_params = {
       .max_elts = 32,
       .elt_size = 128,   /* maximum packet size this pool can hold */
       /* supply .elements / .meta or leave NULL to malloc */
   };

   struct swbus_params params = {
       .name      = "mybus",
       .max_rxqs  = 8,
       .max_subs  = 16,
       .max_pools = 1,
       .pools     = &pool_params,
       /* .flags = RCSW_SWBUS_ASYNC to opt out of sync guarantees */
   };

   struct swbus bus;
   if (NULL == swbus_init(&bus, &params)) {
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
bus-allocated buffer before committing::

   struct swbus_rsrvn res;
   if (OK == swbus_publish_reserve(&bus, &res, sizeof(pkt))) {
       memcpy(res.data, pkt, sizeof(pkt));
       swbus_publish_release(&bus, 0x10, &res, sizeof(pkt));
   }

.. NOTE::

   If :c:func:`swbus_publish_reserve()` succeeds but
   :c:func:`swbus_publish_release()` is never called, the reserved buffer
   will leak until the pool is destroyed. Always pair reserve with release.

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

   /* Non-blocking peek at front entry */
   struct swbus_rxq_ent* front = swbus_rxq_front(rxq);

   /* Timed wait */
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
       free buffer large enough for ``pkt_size``.

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
     - Non-blocking peek at the front entry. Returns ``NULL`` if the queue
       is empty.

   * - :c:func:`swbus_rxq_pop_front()`
     - Release the buffer-pool reference for the front entry and remove it
       from the queue. Must be called after the application is done
       processing a received entry.

Thread Safety
=============

The bus handle itself is protected by an internal :c:struct:`mutex` for
subscription management and by a :c:struct:`rdwrlock` for publish/receive
synchronization (in sync mode). Individual RXQs are :c:struct:`pcqueue`
instances and are independently thread-safe for single-producer / multi-consumer
use. Do not call :c:func:`swbus_rxq_init()` or :c:func:`swbus_subscribe()` /
:c:func:`swbus_unsubscribe()` concurrently with :c:func:`swbus_publish()` on
the same bus instance without external coordination.
