.. SPDX-License-Identifier: MIT

.. _concepts/event-reporting:

===============
Event Reporting
===============

The event reporting (ER) framework is a logging front end. Code reports events
with one set of macros (:c:macro:`ER_DEBUG`, :c:macro:`ER_WARN`, ...); a
*plugin*, chosen at build time with :cmake:variable:`RCSW_CONFIG_ER_PLUGIN`,
decides what happens to them. :ref:`library/er` covers the macros and the
plugins.

Modules
=======

A *module* is the unit logging is grouped and controlled by. Modules are
file-scoped: each ``.c`` file belongs to at most one module, set by defining
two macros before including ``rcsw/er/client.h``:

- :c:macro:`RCSW_ER_MODID`, a 64-bit numeric ID. If a file doesn't define
  one, it gets ``0xFFFFFFFF``, which every such file shares; under LOG4CL they
  are then a single module.
- :c:macro:`RCSW_ER_MODNAME`, a string name. It defaults to the file's
  basename. Build it with :c:macro:`RCSW_ER_MODNAME_BUILDER` so the separator
  suits the plugin (``.`` for LOG4CL, ``_`` for zlog).

Several files can share a module by defining the same ID and name.

A module is *active* once it has been installed, normally with
:c:macro:`RCSW_ER_MODULE_INIT` in the module's initialization function.
Statements in a module that isn't active are suppressed at run time, for
plugins that support per-module control.

.. _concepts/event-reporting/ids:

Module ID allocation
====================

Only the LOG4CL plugin uses module IDs; the others ignore
:c:macro:`RCSW_ER_MODID`. Under LOG4CL, two files with the same ID share a
module, so whichever installs it last decides its level.

RCSW reserves the lower 32 bits (``0x00000000XXXXXXXX``) for its own modules.
Application and library code should use IDs in the upper 32 bits. A simple
scheme for a multi-library project is a 16-bit prefix per library and a 16-bit
per-file number:

.. code-block:: c

   /* Library A owns prefix 0x0001 */
   #define RCSW_ER_MODID  0x0001000000000001ULL  /* library_a/foo.c */
   #define RCSW_ER_MODID  0x0001000000000002ULL  /* library_a/bar.c */

   /* Library B owns prefix 0x0002 */
   #define RCSW_ER_MODID  0x0002000000000001ULL  /* library_b/baz.c */

.. _concepts/event-reporting/levels:

Levels
======

Levels run from most to least severe. Setting a level enables statements at
that level and every more severe one.

.. list-table::
   :header-rows: 1
   :widths: 20 80

   * - Level
     - Statements emitted

   * - ``NONE``
     - None; event reporting is disabled.

   * - ``FATAL``
     - FATAL.

   * - ``ERROR``
     - FATAL, ERROR.

   * - ``WARN``
     - FATAL, ERROR, WARN.

   * - ``INFO``
     - FATAL, ERROR, WARN, INFO. The level a module gets when installed, for
       plugins that have per-module levels.

   * - ``DEBUG``
     - FATAL, ERROR, WARN, INFO, DEBUG.

   * - ``TRACE``
     - All of them.

The constants are ``RCSW_ERL_NONE`` through ``RCSW_ERL_TRACE``.

Compile time and run time
=========================

Levels are applied at two points:

- **Compile time.** ``LIBRA_ERL`` sets the build's level. Statements below it
  are removed by the preprocessor and cost nothing. If it isn't set, every
  level is compiled in. When it is ``FATAL``, no plugin is used at all:
  :c:macro:`ER_FATAL` prints directly with :c:macro:`DPRINTF`.
- **Run time.** Plugins with per-module levels (LOG4CL, zlog) can change a
  module's level while the program runs. They can only filter what was
  compiled in.

Messages
========

Each report is one line. The framework appends ``\r\n`` to the message, so
don't end messages with a newline. The plugin decides the rest of the format;
LOG4CL and simple prefix the module name and level.
