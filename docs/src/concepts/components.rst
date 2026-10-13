.. SPDX-License-Identifier: MIT

.. _concepts/components:

======================
Components and Linking
======================

RCSW is divided into components, roughly one per top-level directory under
``include/rcsw``. It can be built as one monolithic library or as one library
per component; consumers link ``rcsw::rcsw`` or the component targets
accordingly (see :ref:`startup/consumer`). Every component uses ``core`` and
``al``, and most also depend on a few others. Those dependencies matter most
in a shared component build, because every shared library must resolve its
own symbols when it is linked.

Dependencies between components
===============================

Every component needs ``core`` (status codes, :c:func:`rcsw_alloc`) and
``al`` (types, clocks). Every component that logs (algorithm, console, ds,
multithread, swbus and tool) also needs the ER plugin it was built with. The
other link-time dependencies are:

.. list-table::
   :header-rows: 1
   :widths: 20 25 55

   * - Component
     - Needs
     - Because

   * - stdio
     - `eyalroz/printf <https://github.com/eyalroz/printf>`_
     - The ``stdio_*printf()`` family wraps it. It is linked into stdio
       privately and is not part of RCSW's API.

   * - er, LOG4CL plugin
     - ds
     - The installed modules are kept in an :c:struct:`llist`.

   * - er, ``simple`` plugin
     - stdio
     - The plugin prints through :c:func:`stdio_printf`.

   * - utils
     - al
     - :c:func:`utils_ts_make_abs` reads ``clock_realtime()``.

   * - algorithm
     - utils
     - The sorts swap elements with :c:func:`utils_elt_swap`.

   * - ds
     - algorithm, utils
     - :c:struct:`darray` sorts and searches with the ``qsort_*`` and
       ``bsearch_*`` functions; the matrix transposes use
       :c:func:`utils_elt_swap`, and :c:struct:`adjmatrix` edge queries use
       :c:func:`utils_zchk`.

   * - multithread
     - ds, utils
     - :c:struct:`pcqueue` is built on :c:struct:`fifo` (an
       :c:struct:`rbuffer`) and :c:struct:`mpool` on :c:struct:`llist`;
       timed waits convert timeouts with :c:func:`utils_ts_make_abs`.

   * - swbus
     - multithread, ds
     - Packets live in :c:struct:`mpool` buffers and are queued on
       :c:struct:`pcqueue` RXQs, under a :c:struct:`mutex` and
       :c:struct:`rdwrlock`; subscriptions are an :c:struct:`llist`.

   * - tool (grind)
     - utils, and the ER plugin's ``printf``
     - Timing uses the ``utils_ts_*`` helpers. Reports are written with
       :c:macro:`DPRINTF`, so with the ``simple`` plugin this is stdio as
       well.

   * - console (minimon)
     - stdio, utils, version
     - Its interactive stream is :c:func:`stdio_putchar` /
       :c:func:`stdio_getchar`; NMEA framing uses :c:func:`utils_xchks8`; it
       prints build information through the version accessors at start-up.

.. NOTE::

   With the LOG4CL plugin, ``ds`` and ``er`` depend on each other: the data
   structures log through LOG4CL, and LOG4CL keeps its modules in an
   :c:struct:`llist`. A shared *component* build therefore can't resolve
   either library on its own; link them together, or use a monolithic or
   static build.

:ref:`library/platforms` lists which components exist on each platform.

Static and shared builds
========================

:cmake:variable:`RCSW_CONFIG_LIBTYPE` selects the library type. In a shared
build only symbols marked :c:macro:`RCSW_API` are exported; everything else,
including anything marked :c:macro:`RCSW_LOCAL`, is hidden. ``RCSW_API``
expands to a default-visibility attribute when ``RCSW_EXPORTS`` is defined
(while building RCSW itself) and to nothing otherwise.

Two consequences for anyone changing RCSW:

- A ``static inline`` function in a public header is compiled into the
  caller's code, so everything it calls must be exported. Moving the body
  into a ``.c`` file and declaring it ``RCSW_API`` is usually simpler.
- Data objects (``extern const`` tables, vtables such as
  :c:var:`llist_iter_ops`) need ``RCSW_API`` too.

Character output
================

:cmake:variable:`RCSW_CONFIG_STDIO_PUTCHAR` names the function stdio uses to
emit one character, and :cmake:variable:`RCSW_CONFIG_STDIO_GETCHAR` the one it
reads with. Where that function lives depends on the build:

- **Shared build**: the stdio library must resolve the function when it is
  linked, before any application exists. Use a libc function (the default,
  ``putchar`` / ``getchar``).
- **Static build**: the function is resolved when the application is linked,
  so it can be one the application provides, such as a board support
  package's UART routine or a test harness's capture function.

The bundled printf library expects a ``putchar_()`` function; RCSW's stdio
defines it to forward to :c:func:`stdio_putchar`, so applications don't
provide it.
