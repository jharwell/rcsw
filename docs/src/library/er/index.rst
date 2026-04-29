.. _library/er:

====================
Event Reporting (ER)
====================

The ER framework provides a common logging front-end that can be backed by
any of several *plugins*. Application code uses a single set of macros
(``ER_DEBUG()``, ``ER_WARN()``, etc.) regardless of which plugin is active;
the plugin is selected at build time via :cmake:variable:`RCSW_CONFIG_ER_PLUGIN`.

Concepts
========

- **Module** — The unit into which related logging is grouped. Modules are
  file-scoped: each ``.c`` file corresponds to at most one module. A module
  is identified by a numeric UUID (:c:macro:`RCSW_ER_MODID`) and a string
  name (:c:macro:`RCSW_ER_MODNAME`).

- **Active module** — A module is active if
  ``RCSW_ER_PLUGIN_INSMOD()`` has been previously called for it (for
  plugins that support install/enable). Logging statements in an inactive
  module are suppressed at runtime.

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

Module ID Allocation
====================

Module IDs are 64-bit integers. RCSW reserves the lower 32 bits
(``0x00000000XXXXXXXX``) for its own internal modules. Application and
library code should use IDs in the upper 32 bits.

A simple allocation strategy for a multi-library project is to assign each
library a unique upper 16-bit prefix, and use the lower 16 bits within
that library for per-file IDs:

.. code-block:: c

   /* Library A owns prefix 0x0001 */
   #define RCSW_ER_MODID  0x0001000000000001ULL  /* library_a/foo.c */
   #define RCSW_ER_MODID  0x0001000000000002ULL  /* library_a/bar.c */

   /* Library B owns prefix 0x0002 */
   #define RCSW_ER_MODID  0x0002000000000001ULL  /* library_b/baz.c */

ID uniqueness is only required for the ``LOG4CL`` plugin; other plugins
ignore :c:macro:`RCSW_ER_MODID` entirely. Collisions in LOG4CL cause both
files' log output to be controlled by whichever module was installed last
under that ID.

.. _er-levels:

Levels
======

Levels are ordered from most to least severe. Setting a module's level to
``X`` enables all statements at severity ``X`` and above.

.. list-table::
   :header-rows: 1
   :widths: 15 85

   * - Level
     - Behavior

   * - ``NONE``
     - All event reporting disabled.

   * - ``FATAL``
     - Only FATAL events emitted.

   * - ``ERROR``
     - [FATAL, ERROR] emitted.

   * - ``WARN``
     - [FATAL, ERROR, WARN] emitted.

   * - ``INFO``
     - [FATAL, ERROR, WARN, INFO] emitted. Default after
       ``RCSW_ER_PLUGIN_INSMOD()`` for plugins that support levels.

   * - ``DEBUG``
     - [FATAL, ERROR, WARN, INFO, DEBUG] emitted.

   * - ``TRACE``
     - All events emitted. Highest verbosity.

Levels can be controlled at two points:

- **Compile-time**: Statements below the compile-time threshold are
  removed by the preprocessor (zero overhead). Set via
  ``LIBRA_ERL_LVL`` at build time.
- **Run-time**: For plugins that support it (LOG4CL, zlog), levels can
  be adjusted per module while the application is running.

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
     - No
     - Yes
     - ``simple`` uses :c:func:`stdio_printf()`.

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

   * - Thread safety (level check)
     - N/A
     - Yes\ :sup:`†`
     - N/A (zlog-internal)
     - \ :sup:`†` LOG4CL level checks are thread-safe as long as module
       installation/removal is not done concurrently.

   * - Best for
     - Bare-metal, bootstraps, no OS
     - Embedded systems, moderate complexity
     - Linux, complex routing needs
     -
