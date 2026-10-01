.. SPDX-License-Identifier: MIT

.. _concepts/components:

======================
Components and Linking
======================

RCSW is divided into components, roughly one per top-level directory under
``include/rcsw``. It can be built as one monolithic library or as one library
per component; consumers link ``rcsw::rcsw`` or the component targets
accordingly (see :ref:`startup/consumer`). Most components only depend on the
core definitions, but a few depend on each other. Those dependencies matter
most in a shared component build, because every shared library must resolve
its own symbols when it is linked.

Dependencies between components
===============================

The cross-component dependencies are:

.. list-table::
   :header-rows: 1
   :widths: 25 25 50

   * - Component
     - Needs
     - Because

   * - stdio
     - `eyalroz/printf <https://github.com/eyalroz/printf>`_
     - The ``stdio_*printf()`` family wraps it. It is linked into stdio
       privately and is not part of RCSW's API.

   * - console (minimon)
     - stdio, version
     - Its interactive stream is :c:func:`stdio_putchar` /
       :c:func:`stdio_getchar`; it prints build information through the
       version accessors at start-up.

   * - er, with the ``simple`` plugin
     - stdio
     - The plugin prints through :c:func:`stdio_printf`.

   * - tool (grind)
     - the ER plugin's ``printf``
     - Reports are written with :c:macro:`DPRINTF`, so with the ``simple``
       plugin this is stdio as well.

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
