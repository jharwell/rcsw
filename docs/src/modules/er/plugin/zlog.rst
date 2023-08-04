The `zlog <https://github.com/HardySimpson/zlog/tree/master>`_ plugin.

In this plugin, each source file within RCSW and of each project which links
with RCSW can define a logging "module"; modules are file-based, and therefore
you can't have multiple modules/loggers in a single file. If you architect your
projects well, this should not be a burdensome restriction. You *can* split a
logging module across several files if you want by defining
:c:macro:`RCSW_ER_MODNAME` equivalently.

Configuration can be controlled via the zlog configuration file or via the zlog
API as expected; RCSW does not interfere with/wrap that functionality--only the
bits necessary to use logging macros.

Notes:

- Because this is a 3rd party plugin, you will have to point RCSW at the headers
  and the libraries by installing them to ``LIBRA_DEPS_PREFIX``. See
  :ref:`LIBRA<libra:usage/capabilities>` for details.

- Because zlog does not come with a TRACE level, which RCSW requires, RCSW adds
  one as documented in the zlog developer docs. In order to get TRACE events to
  come out as expected, you need to ensure you have the following in your
  ``.conf`` file::

    [levels]
    # 2023-11-13 [JRH]: zlog doesn't have a TRACE level by default, so we
    # add it here. The value MUST match the value specified in the zlog ER
    # plugin, or things won't work correctly.
    TRACE = 10

  There are not any other restrictions/requirements on the ``.conf`` file.

This plugin is useful in:

- Linux targets and/or targets with filesystem.

- Systems where you want to have multiple modules, only some of which should
  be enabled, but *and* you want hierarchical logging power.

Plugin Configuration Details

.. list-table::
   :header-rows: 1
   :widths: 20 80

   * - Configuration Item

     - Notes

   * - :c:macro:`RCSW_ER_PLUGIN_PRINTF`

     - Defined as ``printf()``.

   * - :c:macro:`RCSW_ER_PLUGIN_INIT()`

     - Defined as ``zlog_init()``. Idempotency not mentioned in zlog docs for
       ``zlog_init()``, so not guaranteed. Takes the name of the ``.conf`` file
       as an argument.

   * - :c:macro:`RCSW_ER_PLUGIN_DEINIT()`

     - Defined as ``zlog_fini()``. Idempotency not mentioned in zlog docs for
       ``zlog_fini()``, so not guaranteed. Takes no arguments.

   * - :c:macro:`RCSW_ER_PLUGIN_REPORT()`

     - None.

   * - :c:macro:`RCSW_ER_PLUGIN_INSMOD`

     - Idempotent/not used by this plugin.

   * - :c:macro:`RCSW_ER_PLUGIN_LVL_CHECK`

     - None.

   * - :c:macro:`RCSW_ER_MODNAME`

     - The name of the module. Cannothave dots (``.``) in the name of the
       defined loggers. Use ``_`` instead, as zlog expects. See also
       :c:macro:`RCSW_ER_MODNAME_BUILDER`.

   * - :c:macro:`RCSW_ER_MODID`

     - Not used by this plugin.
