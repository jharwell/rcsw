The simple bare-bones logger.

This plugin uses RCSW's built-in :c:func:`stdio_printf()` and minimal stdlib
implementation, to provide the necessary logging functionality in environments
where stdlib is not available, and/or using stdlib hogs too much space.

In this plugin, each source file within RCSW and of each project which links
with RCSW can define a logging "module"; modules are file-based, and therefore
you can't have multiple modules/loggers in a single file. If you architect your
projects well, this should not be a burdensome restriction.  You *can* split a
logging module across several files if you want by defining
:c:macro:`RCSW_ER_MODNAME` equivalently.

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

  <RCSW_ER_MODNAME> [LVL] <message>

``LVL`` is one of [FATAL, ERROR, INFO, WARN, DEBUG, TRACE], and
``<message>`` is the rendered message. :c:macro:`RCSW_ER_MODNAME` defines the
logical name of the module.


This plugin is useful in:

- Bare metal environments such as bootstraps without an OS.

- Bare metal hardware validation tests.


Plugin Configuration Details

.. list-table::
   :header-rows: 1
   :widths: 20 80

   * - Configuration Item

     - Notes

   * - :c:macro:`RCSW_ER_PLUGIN_PRINTF`

     - Defined as :c:func:`stdio_printf()`.

   * - :c:macro:`RCSW_ER_PLUGIN_INIT()`

     - Idempotent/not used by this plugin.

   * - :c:macro:`RCSW_ER_PLUGIN_DEINIT()`

     - Idempotent/not used by this plugin.

   * - :c:macro:`RCSW_ER_PLUGIN_REPORT()`

     - None.

   * - :c:macro:`RCSW_ER_PLUGIN_INSMOD`

     - Idempotent/not used by this plugin.

   * - :c:macro:`RCSW_ER_PLUGIN_LVL_CHECK`

     - Not used by this plugin/always ``true``.

   * - :c:macro:`RCSW_ER_MODNAME`

     - The name of the module. Can have any format; it can be convenient to use
       a hierarchical format such as ``foo.bar.baz`` for interoperability with
       other plugins. See also :c:macro:`RCSW_ER_MODNAME_BUILDER`.

   * - :c:macro:`RCSW_ER_MODID`

     - Not used by this plugin.
