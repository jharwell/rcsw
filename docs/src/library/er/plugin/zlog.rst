The `zlog <https://github.com/HardySimpson/zlog/tree/master>`_ plugin.

In this plugin, each source file can define one logging module; modules are
file-scoped, so you cannot have multiple modules in a single file. You *can*
share a module name across files by defining :c:macro:`RCSW_ER_MODNAME`
identically in each.

Configuration is controlled via the zlog ``.conf`` file or the zlog API;
RCSW does not wrap that functionality beyond what is necessary to use the
logging macros.

Notes:

- Because zlog is a third-party library, you must install its headers and
  libraries to ``CMAKE_INSTALL_PREFIX`` (or make them accessible via your
  package manager) before building RCSW with this plugin.

- zlog does not define a TRACE level by default. RCSW adds one as described
  in the zlog developer documentation. Add the following to your ``.conf``
  file to enable TRACE output::

    [levels]
    TRACE = 10

  The numeric value **must** match the value defined in the RCSW zlog ER
  plugin; mismatches will cause TRACE events to be silently dropped or
  misrouted.

- Module names **must not contain dots** (``.``). Use underscores (``_``)
  instead, as zlog expects. Use :c:macro:`RCSW_ER_MODNAME_BUILDER` to
  construct names portably; it handles the substitution automatically.

This plugin is useful for:

- Linux targets and other targets with a filesystem.
- Systems requiring hierarchical logging or flexible output routing
  (files, syslog, etc.).

Plugin Configuration Details

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Configuration Item
     - Notes

   * - :c:macro:`RCSW_ER_PLUGIN_PRINTF`
     - Defined as ``printf()``.

   * - :c:macro:`RCSW_ER_PLUGIN_INIT()`
     - Defined as ``zlog_init()``. Takes the path to the ``.conf`` file as
       its argument. Idempotency is not guaranteed by zlog; avoid calling
       more than once.

   * - :c:macro:`RCSW_ER_PLUGIN_DEINIT()`
     - Defined as ``zlog_fini()``. Takes no arguments. Idempotency is not
       guaranteed by zlog.

   * - :c:macro:`RCSW_ER_PLUGIN_REPORT()`
     - Handled internally by zlog.

   * - :c:macro:`RCSW_ER_PLUGIN_INSMOD`
     - Not used by this plugin (zlog manages categories via ``.conf``).

   * - :c:macro:`RCSW_ER_PLUGIN_LVL_CHECK`
     - Handled internally by zlog; thread safety is zlog's responsibility.

   * - :c:macro:`RCSW_ER_MODNAME`
     - Must not contain dots; use ``_`` instead. See
       :c:macro:`RCSW_ER_MODNAME_BUILDER`.

   * - :c:macro:`RCSW_ER_MODID`
     - Not used by this plugin.
