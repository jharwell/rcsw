.. SPDX-License-Identifier: MIT

.. _concepts/data-structures:

===============
Data Structures
===============

The data structures in :ref:`library/ds` share one set of conventions. This
page covers them once; the API reference for each structure only notes where
it differs.

None of the data structures are thread-safe. Callers that share one between
threads must lock around every call; see :ref:`concepts/concurrency`.

Configuration
=============

Each structure ``xx`` is created by ``xx_init(handle, &config)`` with a
``struct xx_config``. The common fields are:

.. list-table::
   :header-rows: 1
   :widths: 20 80

   * - Field
     - Meaning

   * - ``elt_size``
     - Size of one element in bytes.

   * - ``max_elts``
     - Maximum number of elements. Structures that can grow accept -1 for no
       limit; see :ref:`concepts/memory-model` for what that rules out.

   * - ``elements``, ``meta``
     - Caller-provided storage, used only with the matching ``RCSW_NOALLOC_*``
       flag. See :ref:`concepts/memory-model`.

   * - ``cmpe``
     - Element comparator, returning <0, 0 or >0 like ``strcmp()``. Required
       by sorting, searching and sorted insertion. Structures that don't need
       it to work at all accept ``NULL``, which disables those operations.

   * - ``printe``
     - Prints one element. Only used by ``xx_print()``; don't call
       ``xx_print()`` on a structure created without one.

   * - ``flags``
     - The memory flags above plus the structure's own flags. Each structure's
       ``flags`` member lists what it accepts; others are ignored.

Elements
========

Adding an element copies ``elt_size`` bytes from the pointer you pass. The
structure owns the copy, so the original can be reused or freed immediately.
Functions that return an element return a pointer into the structure's
storage, valid until that element is removed or the structure is destroyed.

The exception is a linked list created with
:c:macro:`RCSW_DS_LLIST_DB_DISOWN` or :c:macro:`RCSW_DS_LLIST_DB_PTR`, which
stores your pointers instead of copies and never frees what they point to.

``xx_map(xx, f)`` calls ``f`` on every element, which may modify it in place.
``xx_inject(xx, f, result)`` calls ``f(element, result)`` on every element,
so ``f`` can accumulate a sum, count or other result into ``result``.

Iterators
=========

A :c:struct:`ds_iterator` walks a structure without changing it. It is a value
the caller owns (usually on the stack), so any number of iterators can walk the
same structure at once. Structures that support iteration (currently
:c:struct:`llist`, :c:struct:`darray` and :c:struct:`rbuffer`) provide an
``xx_iter_init()``; after it, call :c:func:`ds_iter_next` until it returns
``NULL``. An optional ``classify`` predicate skips elements it returns false
for. :c:struct:`rbuffer` iterates forward only.

Flags
=====

Flags are bits in a ``uint32_t``. The memory flags in ``rcsw/core/flags.h``
use the lowest bits; module flags start at ``RCSW_MODFLAGS_SHIFT`` and are
defined by each module. Data structure flags are in ``rcsw/ds/ds.h``, and
modules built on top of the data structures start theirs at
``RCSW_DS_EXTFLAGS_SHIFT``.

Binary search trees
===================

The tree types share one implementation, :c:struct:`bstree`:

- A plain :c:struct:`bstree` is an unbalanced binary search tree.
- With :c:macro:`RCSW_DS_BSTREE_RB` it rebalances as a red-black tree.
- The order-statistics tree (``ostree_*`` functions) and the interval tree
  (``inttree_*`` functions) are red-black trees with extra fields per node.
  Create them with :c:func:`ostree_init` / :c:func:`inttree_init`; their
  handle is still a ``struct bstree``.

The ``ostree_*`` and ``inttree_*`` functions wrap the operations intended for
those trees; use them rather than the ``bstree_*`` equivalents.
