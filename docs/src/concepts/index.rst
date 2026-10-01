.. SPDX-License-Identifier: MIT

.. _concepts:

========
Concepts
========

How RCSW works and why, independent of any one module. Read these before the
module reference pages; the API documentation links back here instead of
repeating them.

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Page
     - Covers

   * - :ref:`concepts/memory-model`
     - Who provides the memory for a handle, its data and its metadata;
       sizing caller-provided memory; zeroing; alignment.

   * - :ref:`concepts/error-handling`
     - ``status_t``, ``errno``, function precondition checking, and the
       ``goto error`` idiom.

   * - :ref:`concepts/components`
     - How RCSW is split into components, what depends on what, and what
       changes in a shared-library build.

   * - :ref:`concepts/event-reporting`
     - Modules, IDs, levels, and compile-time versus run-time filtering in the
       ER framework.

   * - :ref:`concepts/data-structures`
     - Conventions shared by every data structure: configuration, callbacks,
       element ownership, iterators, flags, and the binary search tree family.

   * - :ref:`concepts/concurrency`
     - What is and isn't thread-safe, relative versus absolute timeouts, and
       values that are only snapshots.

.. toctree::
   :maxdepth: 1
   :hidden:

   memory-model
   error-handling
   components
   event-reporting
   data-structures
   concurrency
