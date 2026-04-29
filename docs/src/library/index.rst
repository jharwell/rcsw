.. _libraries:

=========
Libraries
=========

RCSW (Robot Control Software) is a portable C library of foundational
building blocks — data structures, synchronization primitives, logging, and
IPC — designed for use from bare-metal embedded targets up to Linux/POSIX
environments.

All modules come with a thorough test suite; the tests also serve as
worked usage examples.

.. _library-platform-matrix:

Platform Availability
=====================

Not all modules are available on all targets. The table below summarizes
which modules are available under each :cmake:variable:`RCSW_BUILD_FOR` setting.

.. list-table::
   :header-rows: 1
   :widths: 30 35 35

   * - Module
     - ``POSIX``
     - ``BAREMETAL``

   * - :ref:`library/ds`
     - ✓
     - ✓ (heap-using ops require allocator)

   * - :ref:`library/algorithm`
     - ✓
     - ✓

   * - :ref:`library/er`
     - ✓ (all plugins)
     - ✓ (``simple`` plugin only without libc)

   * - :ref:`library/stdio`
     - ✓ (optional; disable with :cmake:variable:`RCSW_CONFIG_NO_STDIO`)
     - ✓ (primary use case)

   * - :ref:`library/multithread`
     - ✓
     - ✗ (requires POSIX threads)

   * - :ref:`library/multiprocess`
     - ✓ (requires MPI installation)
     - ✗

   * - :ref:`library/swbus`
     - ✓
     - ✗ (requires multithread)

   * - :ref:`library/utils`
     - ✓
     - ✓

   * - :ref:`library/boot`
     - ✗ (bare-metal only)
     - ✓

.. toctree::
   :maxdepth: 1
   :hidden:

   algorithm
   boot
   er/index
   ds
   multithread
   multiprocess
   swbus
   stdio
   utils
