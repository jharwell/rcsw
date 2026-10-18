.. SPDX-License-Identifier: MIT

.. _concepts/memory-model:

============
Memory Model
============

Every RCSW module that owns memory lets the caller decide, per region, whether
the library allocates it or the caller provides it. This makes the same code
usable with ``malloc()`` on Linux and with no heap at all on bare metal.

Regions
=======

A module manages up to three regions. Each has a flag; passing the flag means
"I am providing this region".

.. list-table::
   :header-rows: 1
   :widths: 15 25 60

   * - Region
     - Flag
     - Caller provides

   * - Handle
     - :c:macro:`RCSW_NOALLOC_HANDLE`
     - The ``struct xx`` itself, passed as the first argument to
       ``xx_init()``. Without the flag that argument is ignored and the
       library allocates the handle.

   * - Data
     - :c:macro:`RCSW_NOALLOC_DATA`
     - Storage for the elements, passed in ``config.elements``. Without the
       flag the field is ignored.

   * - Metadata
     - :c:macro:`RCSW_NOALLOC_META`
     - Per-element bookkeeping such as list or tree nodes, passed in
       ``config.meta``. Without the flag the field is ignored. Not usable with
       an unbounded structure (``max_elts = -1``).

:c:macro:`RCSW_NOALLOC_ALL` sets all three. The flags are independent: you can
provide a handle and let the library allocate the elements. Each module's
``flags`` field lists which flags it accepts; a module that has no data or
metadata region ignores those flags. Building with
:cmake:variable:`RCSW_CONFIG_NOALLOC` implies all three flags for every
module.

``xx_destroy()`` frees only the regions the library allocated. A region the
caller provided is left alone, and remains the caller's to release.

Sizing caller-provided memory
=============================

Each region the caller provides must be large enough for everything the
library will put in it:

- **Handle**: at least ``sizeof(struct xx)``.
- **Data**: usually ``xx_element_space(max_elts, elt_size)`` bytes.
- **Metadata**: usually ``xx_meta_space(max_elts)`` bytes, for modules that
  have metadata (lists, trees, hashmaps, memory pools, multififo, swbus).
  Contiguous structures such as :c:struct:`darray` and :c:struct:`rbuffer`
  have none.

A few sizing functions take different arguments because the structure is
sized differently:

- :c:func:`hashmap_element_space()`
- :c:func:`hashmap_meta_space`
- :c:func:`inttree_element_space`
- :c:func:`multififo_meta_space`
- :c:func:`swbus_meta_space`

Each one's API documentation lists its arguments.

The sizing functions include any internal bookkeeping (allocation maps,
sentinel nodes), so the result can be larger than ``max_elts * elt_size``.
They are ``static inline`` functions, not constant expressions; to use a
static buffer, size it generously and check it at run time, as the
:ref:`library/ds/quickstart` does.

.. WARNING::

   Undersized caller memory corrupts whatever follows it. The library cannot
   detect it, and with zeroing enabled the damage happens inside ``xx_init()``
   itself, far from the eventual symptom.

Zeroing
=======

:c:macro:`RCSW_ZALLOC` asks for memory to be zeroed before use: library
allocations use ``calloc()``, and caller-provided regions are cleared with
``memset()`` for their full size. :cmake:variable:`RCSW_CONFIG_ZALLOC` turns
this on everywhere.

Because caller-provided regions are cleared for their full size, zeroing turns
an undersized buffer from a latent bug into an immediate overwrite of the
neighbouring memory.

Layout & Alignment
==================

Many RCSW data structures can run entirely out of memory you provide
(:c:macro:`RCSW_NOALLOC_HANDLE`, :c:macro:`RCSW_NOALLOC_DATA`,
:c:macro:`RCSW_NOALLOC_META`). This page explains how RCSW lays out that
memory, and what it guarantees about alignment, so the same code is correct on
x86, ARM, RISC-V and smaller embedded targets.

The short version:

- Pass memory aligned to at least :cmake:variable:`RCSW_CONFIG_PTR_ALIGN`
  (any ``dptr_t`` array is), sized with the structure's ``*_element_space()`` /
  ``*_meta_space()`` functions.
- Set ``elt_size`` to ``sizeof`` your element type.
- Every internal node and element is then naturally aligned, at every
  optimization level, with every conforming compiler.

Element ``i`` of a data structure's element pool is at ``i * elt_size`` from the
start of the pool, exactly like a C array. With ``elt_size == sizeof(T)``, every
element therefore has ``T``'s alignment, because ``sizeof(T)`` is always a
multiple of ``_Alignof(T)``. An ``elt_size`` that is not a multiple of
:cmake:variable:`RCSW_CONFIG_PTR_ALIGN` is allowed; elements then have their
type's alignment but not the stronger
:cmake:variable:`RCSW_CONFIG_PTR_ALIGN`.

The same applies to :c:struct:`mpool` chunks, and therefore to :c:struct:`swbus`
packet buffers: chunk ``i`` is at ``i * elt_size``, so a pool with 170-byte
elements hands out buffers that are only 2-byte aligned. Choose element sizes
that are multiples of the alignment your data needs, or copy packets out before
accessing them as structs.


``dptr_t`` and ``RCSW_CONFIG_PTR_ALIGN``
----------------------------------------

:cmake:variable:`RCSW_CONFIG_PTR_ALIGN` is the alignment RCSW promises for the
start of every element and node in RCSW-managed memory. ``dptr_t`` is an
unsigned integer type with exactly that alignment, so a ``dptr_t[]`` buffer is
always suitably aligned for use as caller-provided storage. Choose the
alignment of the most strictly aligned type you store; e.g., 8 on 64-bit
targets that store pointers, or structs containing them.

RCSW's own list and tree nodes contain pointers, so with caller-provided
metadata (:c:macro:`RCSW_NOALLOC_META`) the value must also be at least the
target's pointer alignment. Below that (e.g., 4 on a 64-bit target), every
node in the metadata region has misaligned pointers: undefined behavior that
works on x86 but traps or slows down on stricter targets. Metadata the
library allocates itself is not affected.

The library checks at build time that the target's ABI really gives ``dptr_t``
that alignment; on 32-bit x86, for example, ``uint64_t`` is only 4-byte aligned
in structs, so 8 is rejected there.

Why internal structs are not packed
-----------------------------------

Nodes and other internal structs (:c:struct:`bstree_node`, ``struct
ostree_node``, ``struct inttree_node``, ``struct llist_node``,
:c:struct:`hashnode`, :c:struct:`allocm_entry`, etc) are declared with natural
alignment, and with ``aligned(sizeof(dptr_t))`` where an array of them must
keep every element aligned. They are deliberately **not** ``packed``.

``packed`` removes padding by placing each member immediately after the previous
one. ``aligned`` only raises the alignment of the struct as a whole; it does not
restore the padding *inside* it. For example, if in ``bstree_node`` the key is 4
bytes, with ``packed`` the ``data``, ``left``, ``right`` and ``parent`` pointers
would start at offsets 4, 12, 20 and 28: misaligned on every 64-bit target. A
misaligned pointer access is undefined behavior in C; it is slow on x86, and on
many ARM, RISC-V and MIPS cores it traps.

Without ``packed`` the compiler inserts 4 bytes of padding after the key, and
the pointers land at 8, 16, 24 and 32. The node is the same size either way (48
bytes on 64-bit targets), because ``aligned(8)`` already rounded the packed size
up to a multiple of 8.

A natural-alignment layout is fixed by the platform ABI (System V psABI for x86
and x86-64, AAPCS for ARM, the RISC-V psABI, and so on): each member is placed
at the next offset that is a multiple of its alignment, and the struct is padded
to a multiple of its largest member alignment. This is what makes separately
compiled code and libraries interoperate, so:

- It does not change with ``-O0``\/``-O2``\/``-Os`` or LTO;
- GCC and Clang produce the same layout for the same target;
- The only thing that can change it is a different target ABI, which is exactly
  when you rebuild the library anyway.

The shared tree node prefix
---------------------------

The binary search tree code is shared by plain, red-black, order-statistic
(``struct ostree_node``) and interval (``struct inttree_node``) trees, and
walks every node through a ``struct bstree_node *``. The variant nodes therefore
begin with the same members as :c:struct:`bstree_node` (``key``, ``data``,
``left``, ``right``, ``parent``, ``red``), and their extra member (``count`` /
``max_high``) overlays ``bstree_node.tmp``. All three nodes have the same size,
so they can share one node pool, with a stride of ``sizeof(struct
bstree_node)``.

This is enforced at build time: a change that breaks the overlay fails to
compile instead of corrupting trees at run time.
