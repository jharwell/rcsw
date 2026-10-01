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
- **Data**: ``xx_element_space(max_elts, elt_size)`` bytes.
- **Metadata**: ``xx_meta_space(max_elts)`` bytes, for modules that have
  metadata (lists, trees, hashmaps, memory pools). Contiguous structures such
  as :c:struct:`darray` and :c:struct:`rbuffer` have none.

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

Alignment
=========

Caller-provided data and metadata regions are typed ``dptr_t*``. ``dptr_t``
is an unsigned integer whose alignment equals
:cmake:variable:`RCSW_CONFIG_PTR_ALIGN`, so an array of it is aligned for the
data RCSW will store there. Declare buffers as ``dptr_t`` arrays, not
``uint8_t`` arrays; a ``uint8_t`` buffer has no alignment guarantee.

The default is the pointer size on known architectures (8 on 64-bit, 4 on
32-bit) and 1 otherwise. To override it, choose the alignment of the most
strictly aligned type stored in RCSW-managed memory. If the elements you store
contain pointers, that is the pointer size (8 on 64-bit targets). Some ABIs
align ``uint64_t`` to 4 bytes (32-bit x86), and 8 is rejected there. Targets
that trap on unaligned access (some ARM cores) fault if a stored struct ends up
less aligned than the compiler assumes.

RCSW's own internal records (list nodes, tree nodes, allocation map entries)
are packed and aligned to ``sizeof(dptr_t)``, so they keep this alignment when
laid out back to back in caller memory.
