.. SPDX-License-Identifier: MIT

.. _library/er:

====================
Event Reporting (ER)
====================

The ER framework provides a common logging front-end that can be backed by
any of several *plugins*. Application code uses a single set of macros
(``ER_DEBUG()``, ``ER_WARN()``, etc.) regardless of which plugin is active;
the plugin is selected at build time via :cmake:variable:`RCSW_CONFIG_ER_PLUGIN`.

See :ref:`concepts/event-reporting` for modules, module IDs, levels, and
compile-time versus run-time filtering.

Quickstart
==========

**Step 1** — Near the top of your ``.c`` file, define the module ID and
name, then include the ER client header:

.. code-block:: c

   #define RCSW_ER_MODID   0x0001000000000000ULL
   #define RCSW_ER_MODNAME "myapp.subsystem.component"
   #include "rcsw/er/client.h"

**Step 2** — In the module's initialization function, install the module:

.. code-block:: c

   void mycomponent_init(void) {
       RCSW_ER_MODULE_INIT();
   }

**Step 3** — Use the logging macros anywhere in the file:

.. code-block:: c

   ER_DEBUG("Initialized with n_elts=%zu", n_elts);
   ER_WARN("Buffer nearly full: %zu/%zu", used, capacity);
   ER_FATAL("Unrecoverable error: %d", err);

That's it. ``RCSW_ER_MODULE_INIT()`` is the recommended path — use
``RCSW_ER_INSMOD(id, name)`` only if you need to install multiple modules
from a single initializer that doesn't correspond to a specific file.

.. TIP::

   Not all plugins use both :c:macro:`RCSW_ER_MODID` and
   :c:macro:`RCSW_ER_MODNAME` (see plugin details below). Define both
   regardless to keep your code portable across plugins.

   Use :c:macro:`RCSW_ER_MODNAME_BUILDER` to construct the name string so
   it works correctly with every plugin (e.g., it substitutes ``_`` for
   ``.`` when building for zlog).

Plugins
=======

.. tab-set::

   .. tab-item:: Simple

      .. include:: plugin/simple.rst

   .. tab-item:: LOG4CL

      .. include:: plugin/log4cl.rst

   .. tab-item:: zlog

      .. include:: plugin/zlog.rst

   .. tab-item:: Custom

      .. include:: plugin/custom.rst

Plugin Comparison
=================

.. list-table::
   :header-rows: 1
   :widths: 28 12 12 12 36

   * - Feature
     - Simple
     - LOG4CL
     - zlog
     - Notes

   * - Stdlib required
     - No
     - Yes
     - Yes
     - ``simple`` uses :c:func:`stdio_printf()`; LOG4CL uses libc
       ``printf()``.

   * - Per-module enable/disable
     - No
     - Yes
     - Yes
     - ``simple`` has one global compile-time level only.

   * - Runtime level adjustment
     - No
     - Yes
     - Yes
     - ``simple`` is compile-time only.

   * - Hierarchical logging
     - No
     - No
     - Yes
     - zlog supports hierarchical category matching.

   * - File/sink routing
     - No
     - No
     - Yes
     - zlog routes to files, syslog, etc. via ``.conf``.

   * - Thread safety
     - N/A
     - Yes\ :sup:`†`
     - N/A (zlog-internal)
     - \ :sup:`†` On POSIX, LOG4CL serializes module installation, removal,
       lookup and level changes internally. Don't call ``RCSW_ER_DEINIT()``
       while other threads are reporting.

   * - Best for
     - Bare-metal, bootstraps, no OS
     - Embedded systems, moderate complexity
     - Linux, complex routing needs
     -
