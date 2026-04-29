.. SPDX-License-Identifier: MIT

.. _startup:

===========
Using RCSW
===========

.. _startup/consumer:

As a Dependency
===============

The recommended way to consume RCSW in a CMake project is via
`CPM <https://github.com/cpm-cmake/CPM.cmake>`_:

.. code-block:: cmake

   CPMAddPackage(
     NAME rcsw
     GITHUB_REPOSITORY jharwell/rcsw
     VERSION <version>
   )
   target_link_libraries(myapp PRIVATE rcsw::rcsw)

Or, if you have installed RCSW via ``cmake --install``:

.. code-block:: cmake

   find_package(rcsw REQUIRED)
   target_link_libraries(myapp PRIVATE rcsw::rcsw)

RCSW exports a single CMake target ``rcsw``. All include paths, compile
definitions, and transitive dependencies are propagated automatically via that
target.

.. _startup/dev:

As a Developer
==============

From the root of the repo:

.. code-block:: bash

   cmake --preset debug <ARGS>

``<ARGS>`` is a list of cmake arguments. You can pass any option that LIBRA
supports (see :ref:`libra:main`) or any RCSW option listed below.

.. _startup/cmake:

CMake Configuration
===================

.. NOTE::

   All cmake options with a direct code analogue are prefixed
   ``RCSW_CONFIG_``; options that affect only the build system (e.g.,
   ``RCSW_SUMMARY``) are prefixed ``RCSW_``.

Build Targets
-------------

.. cmake:variable:: RCSW_BUILD_FOR

   :default: POSIX

   Target platform. Valid values:

   - ``POSIX`` — Linux and POSIX-compatible OSes. Enables the full
     feature set including multithread, multiprocess, and swbus.
   - ``BAREMETAL`` — Bare-metal environments without an OS. Enables
     stdlib from the selected compiler. To also strip stdlib, pass
     ``LIBRA_NOSTDLIB`` (see :ref:`LIBRA docs <libra:main>`); this
     restricts which modules are usable. See
     :ref:`library-platform-matrix` for the full breakdown.


.. cmake:variable:: RCSW_CONFIG_LIBTYPE

   :default: STATIC

   Whether to build RCSW as a ``SHARED`` or ``STATIC`` library.

.. cmake:variable:: RCSW_CONFIG_NO_STDIO

   :default: NO

    Exclude the STDIO module from compilation. Useful on POSIX targets
    when you are linking against libc and do not need the printf
    replacement.

.. cmake:variable:: RCSW_SUMMARY

   :default: YES

   Print a summary of all RCSW-specific cmake variables at configure
   time.

Memory Allocation
-----------------

.. cmake:variable:: RCSW_CONFIG_NOALLOC

   :default: NO

   Disallow all dynamic heap allocation. When ``YES``, implies
   :c:macro:`RCSW_NOALLOC_META`, :c:macro:`RCSW_NOALLOC_DATA`, and
   :c:macro:`RCSW_NOALLOC_HANDLE` for all modules that support them.

   You can also set these flags individually at runtime via the
   ``params->flags`` field on each module's ``_params`` struct. The three flags
   are independent: for example, you can supply a caller-allocated handle
   (:c:macro:`RCSW_NOALLOC_HANDLE`) while allowing the library to malloc element
   storage (omit :c:macro:`RCSW_NOALLOC_DATA`).  See :ref:`ds-memory-model` for
   details and ``XX_element_space()`` / ``XX_meta_space()`` for sizing helpers.

.. cmake:variable:: RCSW_CONFIG_ZALLOC

   :default: NO

   Zero-initialize all memory (whether dynamically allocated or caller-supplied)
   before use. See :c:macro:`RCSW_ZALLOC` for full implications.

.. cmake:variable:: RCSW_CONFIG_PTR_ALIGN

   :default: x86: 4; ARM: 1; all others: 1

  Override the pointer alignment used to store all application data
   managed by RCSW.

   Some architectures trap on unaligned accesses (e.g., strict ARM variants);
   others handle them silently or in hardware. RCSW detects the best value for
   known architectures, but falls back to byte alignment (1) for unknown
   targets. Override if you know your target requires or benefits from a
   specific alignment.

   Must be one of [1, 2, 4].

STDIO / printf Replacement
--------------------------

.. cmake:variable:: RCSW_CONFIG_STDIO_PUTCHAR

   :default: ``putchar``

   Name of the ``putchar()``-compatible function RCSW's printf will
   call to emit characters. Must have the same signature as
   ``putchar()``. Override when linking to a custom BSP output
   routine.

.. cmake:variable:: RCSW_CONFIG_STDIO_GETCHAR

   :default: ``getchar``

   Name of the ``getchar()``-compatible function RCSW's printf will call to read
   characters. Must have the same signature as ``getchar()``.

printf() formatting behavior (float support, buffer sizes, precision,
etc.) is controlled by the `eyalroz/printf
<https://github.com/eyalroz/printf>`_ dependency, which RCSW pulls in via
CPM. To override an upstream option, set it in your CMake invocation
before including RCSW — CPM respects already-set cache variables.

Event Reporting
---------------

.. cmake:variable:: RCSW_CONFIG_ER_PLUGIN

   :default: LOG4CL

   The ER plugin to build against. See :ref:`library/er` for plugin
   details and tradeoffs.

.. cmake:variable:: RCSW_CONFIG_ER_PLUGIN_PATH

   :default: (empty)

   Absolute or relative (to the build directory) path to the
   ``#include`` file defining the custom ER plugin macros. Only
   required when ``RCSW_CONFIG_ER_PLUGIN=CUSTOM``. This file is
   **not** installed with RCSW; ensure it is findable by any
   application that links against RCSW. See the Custom plugin tab in
   :ref:`library/er` for required macro definitions.

Instrumentation
---------------

.. cmake:variable:: RCSW_CONFIG_NO_GRIND

   :default: NO

   Compile out the timing/profiling macros
   :c:macro:`RCSW_GRIND_START()`, :c:macro:`RCSW_GRIND_END()`,
   :c:macro:`RCSW_GRIND_COUNT()`, and :c:macro:`RCSW_GRIND_TICK()`.
   Allows timing instrumentation to be left in application source and
   enabled only when profiling is needed, without runtime cost
   otherwise.
