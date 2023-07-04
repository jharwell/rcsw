.. _ln-rcsw-er-plugin:

=======
Plugins
=======

.. _ln-rcsw-er-plugin-simple:

SIMPLE Plugin
-------------

The simple bare-bones logger.

This plugin uses RCSW's built-in :ref:`stdio_printf()` and minimal stdlib
implementation, to provide the necessary logging functionality in environments
where stdlib is not available, and/or using stdlib hogs too much space.

In this plugin, each source file within RCSW and of each project which links
with RCSW can define a logging "module"; modules are file-based, and
therefore you can't have multiple modules/loggers in a single file. If you
architect your projects well, this should not be a burdensome restriction.
The modules in this plugin:

- Are unconditionally enabled and cannot be disabled; that is, each time a
  logging statement is encountered during execution, it is always emitted.

- Do not have a "level" associated with them which determines if a given
  logging statement should be emitted. Put another way, there is only a
  single global module whose level is set at compile time; statements less than
  the compile-time level are compiled out.

- The name of each module is prepended to each logging statement to help
  identify the logging source.

Each emitted logging statement is of the form::

  <RCSW_ER_MODULE_NAME> [LVL] <message>

Where :ref:`RCSW_ER_MODULE_NAME` is the #define defining the name of the
module. If it is not defined, `__FILE_NAME__`` is used. \a LVL is one of FATAL,
ERROR, INFO, WARN, DEBUG, TRACE.

This plugin is useful in:

- Bare metal environments such as bootstraps without an OS.

- Bare metal hardware validation tests.


.. _ln-rcsw-er-plugin-log4cl:

LOG4CL Plugin
-------------

A lighter/simpler version of `<WEBSITE> log4c`_.

In this plugin, each source file within RCSW and of each project which links
with RCSW can define a logging "module"; modules are file-based, and therefore
you can't have multiple modules/loggers in a single file. If you architect your
projects well, this should not be a burdensome restriction.

Each module can be be enabled/disabled independently in a lightweight manner;
unlike log4j loggers, the enable/disable status of one logger/module does not
have any effect on the status of another. That is, the hierarchy is "flat".

This plugin does not provide most features found in log4c, with the exception of
levels: INFO, WARN, DEBUG, TRACE, FATAL. If you want most features from log4c,
use the log4c plugin.

Each emitted logging statement is of the form::

  <RCSW_ER_MODULE_NAME> [LVL] <message>

Where :ref:`RCSW_ER_MODULE_NAME` is the #define defining the name of the
module. If it is not defined, ``__FILE_NAME__`` is used. \a LVL is one of FATAL,
ERROR, INFO, WARN, DEBUG, TRACE.

This is useful in:

- Medium-complexity embedded systems with limited resources

- Systems where you want to have multiple modules, only some of which should
  be enabled, but don't need the hierarchical logging of log4c. In this
  scheme, modules are either enabled or not whether a given module is enabled
  has no effect on other modules. In addition, the name given to a specific
  module is purely for debugging purposes, and has no effect on event
  reporting; you can have multiple modules with the same name and different
  IDs, if you want.


.. _ln-rcsw-er-plugin-custom:

Custom Plugin
-------------

A custom plugin which is defined exactly how you want in terms of modules,
levels, etc. To integrate your plugin with RCSW, you must create a ``.h`` file
with a few ``#define`` macros. Your ``.h`` file must be specified at configure
time via ``-DRCSW_ER_PLUGIN_PATH=/path/to/your/file``.

.. IMPORTANT:: Your ``.h`` file is **NOT** installed/packaged with RCSW, so you
               will need to ensure that it is findable by any applications you
               want to use with RCSW+your custom ER plugin.

Plugin file contents (you can of course have whatever else you want in the
file):

.. list-table::
   :header-rows: 1
   :widths: 10 50 30 10

   * - ``#define``

     - Description

     - Required?

     - Disable Configuration

   * - ``RCSW_ER_PLUGIN_PRINTF``

     - The name of the ``printf()``-like function which has the same signature;
       used to define the :ref:`PRINTF` / :ref:`DPRINTF` macros.

     - No; if omitted, you can't use the :ref:`PRINTF` / :ref:`DPRINTF` macros.

     - ``#define`` as nothing

   * - ``RCSW_ER_PLUGIN_INIT(...)``

     - A framework initialization hook which RCSW will call in its internal
       modules; should be idempotent. Can take any number of arguments of any
       type.

     - No; if omitted then:

       - :ref:`ln-rcsw-er-plugin-log4cl` plugin will not work with RCSW modules.
       - :ref:`ln-rcsw-er-plugin-simple` will still work with RCSW modules.

     - ``#define`` as nothing.

   * - ``RCSW_ER_PLUGIN_SHUTDOWN()``

     - A framework shutdown hook; should be idempotent. Should not take any
       arguments. Not used by RCSW currently.

     - No

     - ``#define`` as nothing.

   * - ``RCSW_ER_PLUGIN_REPORT(lvl, handle, id, name, msg, ...)``

     - The main ER plugin hook. Will be called as part of every
       :ref:`ER_WARN()`, etc. statement. Arguments:

       - ``lvl`` - The level of the statement. See :ref:`ln-rcsw-er-levels` for
         details.

       - ``handle`` - Whatever was returned from ``RCSW_ER_PLUGIN_HANDLE()``.

       - ``id`` - The ID of the current module (file). This will expand to
         nothing if ``RCSW_ER_MODID`` is not defined.

       - ``name`` - The name of the current module (file)

       - ``msg`` - The message string

       - ``...`` - Any additional arguments for the message string

     - Yes

     - N/A

   * - ``RCSW_ER_PLUGIN_INSMOD(id, name)``

     - Install/enable a module with the specified ID and name.

     - No

     - ``#define`` as nothing.

   * - ``RCSW_ER_PLUGIN_HANDLE(id, name)``

     - Get a logger "handle" of some kind which contains the necessary
       information to determine if a given module is enabled. For example, in
       the :ref:`LOG4CL` plugin, the :ref:``log4cl_mod_query()`` function
       serves this purpose.

       If the module with the specified ``id, name`` is not enabled, then the
       handle should be a false-y value, like 0 or NULL.

     - No

     - ``#define`` as nothing.

   * - ``RCSW_ER_PLUGIN_LVL_CHECK(handle, lvl)``

     - Given an active module ``handle``, determine if the statement with the
       specified ``lvl`` should be emitted or not.

     - No

     - ``#define`` as a truth-y value, such as 1.

A few important notes:

- If your plugin uses ``RCSW_ER_MODID``, it should support 64-bit IDs if you
  want to RCSW against it.
