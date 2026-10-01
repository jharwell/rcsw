.. SPDX-License-Identifier: MIT

==========================
Reusable C Software (RCSW)
==========================

.. |ci-master| image:: https://github.com/jharwell/rcsw/actions/workflows/ci.yml/badge.svg?branch=master
.. |ci-devel| image:: https://github.com/jharwell/rcsw/actions/workflows/ci.yml/badge.svg?branch=devel
.. |coverage-master| image:: https://coveralls.io/repos/github/jharwell/rcsw/badge.svg?branch=master
                             :target: https://coveralls.io/github/jharwell/rcsw?branch=master
.. |coverage-devel| image:: https://coveralls.io/repos/github/jharwell/rcsw/badge.svg?branch=devel
                            :target: https://coveralls.io/github/jharwell/rcsw?branch=devel
.. |license| image:: https://img.shields.io/github/license/jharwell/rcsw
                     :target: https://github.com/jharwell/rcsw/blob/master/LICENSE
.. |docs| image:: https://github.com/jharwell/rcsw/actions/workflows/pages.yml/badge.svg?branch=master
                  :target: https://jharwell.github.io/rcsw
.. |maintenance| image:: https://img.shields.io/badge/Maintained%3F-yes-green.svg

:Release: |ci-master| |coverage-master|
:Development: |ci-devel| |coverage-devel|
:Misc: |license| |docs| |maintenance|

RCSW is a C library of building blocks (data structures, algorithms,
synchronization, logging, and the small runtime pieces an embedded project
ends up writing for itself) that runs everywhere from a bare-metal bootloader
with no heap and no libc to a multithreaded Linux application.

Its defining feature is that **you decide where the memory comes from.** Every
module that owns memory can take its handle, its element storage, and its
bookkeeping from the caller instead of ``malloc()``, independently. The same
linked list, hashmap, or memory pool code works with a heap on Linux and
entirely from static buffers on a microcontroller.

Full documentation: https://jharwell.github.io/rcsw

Why RCSW
========

- **No heap required.** Pass ``RCSW_NOALLOC_HANDLE``, ``RCSW_NOALLOC_DATA``
  and/or ``RCSW_NOALLOC_META`` to supply memory yourself, or build with
  ``RCSW_CONFIG_NOALLOC`` to forbid allocation library-wide. Sizing functions
  (``llist_element_space()``, ``llist_meta_space()``, ...) tell you how much
  to reserve.
- **Bare metal and POSIX from one codebase.** The core, data structures,
  algorithms, utilities, logging, and a ``printf()`` replacement work without
  an OS; threading, process management, and the software bus are added on
  POSIX.
- **Alignment handled for you.** Caller buffers are declared as ``dptr_t``
  arrays, sized and aligned by ``RCSW_CONFIG_PTR_ALIGN`` for the target, so
  stored structs don't fault on cores that trap on misaligned access.
- **Configurable argument checking.** Public functions validate their
  arguments with precondition checks that, chosen at build time, return an
  error with ``errno = EINVAL``, ``assert()``, or compile out entirely.
- **Pluggable logging.** One set of macros (``ER_DEBUG()``, ``ER_WARN()``,
  ...) with per-module control, compile-time level filtering that removes
  disabled statements entirely, and a choice of backend: the built-in LOG4CL
  or simple plugins, `zlog <https://github.com/HardySimpson/zlog>`_, or your
  own.
- **Tested.** Every module has unit tests (Catch2), run in CI under
  AddressSanitizer, with coverage tracked.

What's Inside
=============

.. list-table::
   :header-rows: 1
   :widths: 18 62 20

   * - Component
     - Contents
     - Platforms

   * - ``ds``
     - Ringbuffer, FIFO, multi-FIFO, lock-free single-producer/consumer raw
       FIFO, linked list, dynamic array, hashmap, binary heap, binary search
       tree (plain, red-black, order-statistic, interval), static/dynamic/
       adjacency matrices, iterators.
     - All

   * - ``algorithm``
     - Binary search, quicksort, insertion sort, radix sort, edit distance,
       longest common subsequence, matrix-chain ordering.
     - All

   * - ``utils``
     - Checksums (XOR, additive, CRC-32), hashes (FNV-1a, Jenkins, DJB2),
       bit and byte manipulation, register/memory access and hex dumps,
       ``timespec`` arithmetic.
     - All (time: POSIX)

   * - ``stdio``
     - ``printf()`` family, string and memory routines, number conversions:
       enough to print from a bootloader with no libc.
     - All

   * - ``er``
     - Event reporting (logging) front end and plugins.
     - All

   * - ``multithread``
     - Mutex, condition variable, binary and counting semaphores, fair
       reader/writer lock, reference-counted memory pool, producer-consumer
       queue, thread-to-core pinning.
     - POSIX

   * - ``multiprocess``
     - ``fork()``/``exec()`` helper, socket (CPU package) pinning.
     - POSIX

   * - ``swbus``
     - Publish-subscribe software bus with zero-copy publishing.
     - POSIX

   * - ``console``
     - Minimon, an interactive serial monitor for board bring-up: read and
       write memory, load and send data, jump, plus your own commands.
     - Bare metal

   * - ``tool``
     - Grind, lightweight timing and execution-count instrumentation.
     - All (needs libc)

Example
=======

A linked list that never touches the heap. The handle, the elements, and the
list nodes all live in static storage:

.. code-block:: c

   #include "rcsw/ds/llist.h"

   #define N_ELTS 16

   static dptr_t       elements[512 / sizeof(dptr_t)];
   static dptr_t       nodes[1024 / sizeof(dptr_t)];
   static struct llist list;

   status_t setup(void) {
     /* The sizing functions aren't constant expressions: check at run time */
     if (sizeof(elements) < llist_element_space(N_ELTS, sizeof(int)) ||
         sizeof(nodes) < llist_meta_space(N_ELTS)) {
       return ERROR;
     }
     struct llist_config config = {
       .elements = elements,
       .meta     = nodes,
       .elt_size = sizeof(int),
       .max_elts = N_ELTS,
       .flags    = RCSW_NOALLOC_ALL,
     };
     if (NULL == llist_init(&list, &config)) {
       return ERROR; /* errno says why */
     }

     int val = 42;
     return llist_append(&list, &val); /* copies val into the list */
   }

Drop the ``RCSW_NOALLOC_*`` flags (and the buffers) and the same code
allocates from the heap instead.

Getting Started
===============

RCSW builds with CMake on top of `LIBRA <https://github.com/jharwell/libra>`_.
To use it from a CMake project with
`CPM <https://github.com/cpm-cmake/CPM.cmake>`_:

.. code-block:: cmake

   CPMAddPackage(
     NAME rcsw
     GITHUB_REPOSITORY jharwell/rcsw
     VERSION <version>
   )
   target_link_libraries(myapp PRIVATE rcsw::rcsw)

or, after ``cmake --install``:

.. code-block:: cmake

   find_package(rcsw REQUIRED)
   target_link_libraries(myapp PRIVATE rcsw::rcsw)

A component build provides one target per component instead of ``rcsw::rcsw``;
link the ones you use.

To build from a clone:

.. code-block:: bash

   cmake --preset debug -DRCSW_BUILD_FOR=BAREMETAL   # or POSIX (the default)

The most common options:

.. list-table::
   :header-rows: 1
   :widths: 35 65

   * - Option
     - Effect

   * - ``RCSW_BUILD_FOR``
     - ``POSIX`` (default) or ``BAREMETAL``.

   * - ``RCSW_CONFIG_LIBTYPE``
     - ``STATIC`` (default) or ``SHARED``.

   * - ``RCSW_CONFIG_NOALLOC``
     - Never allocate; all memory comes from the caller.

   * - ``RCSW_CONFIG_PTR_ALIGN``
     - Alignment of caller-provided storage: 1, 2, 4 or 8.

   * - ``RCSW_CONFIG_ER_PLUGIN``
     - Logging backend: ``LOG4CL`` (default), ``SIMPLE``, ``ZLOG`` or
       ``CUSTOM``.

   * - ``RCSW_CONFIG_STDIO_PUTCHAR`` / ``_GETCHAR``
     - The character I/O functions ``stdio`` uses, e.g. your UART driver.

See `Using RCSW <https://jharwell.github.io/rcsw/startup.html>`_ for every
option.

Documentation
=============

https://jharwell.github.io/rcsw has:

- **Using RCSW**: installing, linking, and every build option.
- **Concepts**: the memory model, error handling, components and linking,
  event reporting, data structure conventions, and concurrency. Read these
  first; the API reference assumes them.
- **Library**: a page per component, plus the API reference generated from
  the headers.

License
=======

MIT. See `LICENSE <LICENSE>`_.
