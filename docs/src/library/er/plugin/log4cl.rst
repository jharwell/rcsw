A lighter/simpler version of `log4c <https://log4c.sourceforge.net/>`_.

In this plugin, each source file within RCSW and of each project which links
with RCSW can define a logging "module"; modules are file-based, and therefore
you can't have multiple modules/loggers in a single file. If you architect your
projects well, this should not be a burdensome restriction. You *can* split a
logging module across several files if you want by defining
:c:macro:`RCSW_ER_MODNAME` equivalently.

Each module can be enabled/disabled independently in a lightweight manner;
unlike log4j loggers, the enable/disable status of one logger/module does not
have any effect on the status of another. That is, the hierarchy is "flat".

This plugin does not provide most features found in log4c, with the exception of
levels: INFO, WARN, DEBUG, TRACE, FATAL. If you want something with features
comparable to log4c, use the zlog plugin.

Each emitted logging statement is of the form::

  <RCSW_ER_MODNAME> [LVL] <message>

``LVL`` is one of [FATAL, ERROR, INFO, WARN, DEBUG, TRACE], and ``<message>`` is
the rendered message. :c:macro:`RCSW_ER_MODNAME` defines the logical name of the
module.


This plugin is useful in:

- Medium-complexity embedded systems with limited resources.

- Systems where you want to have multiple modules, only some of which should
  be enabled, but don't need the hierarchical logging of log4c. In this
  scheme, modules are either enabled or not whether a given module is enabled
  has no effect on other modules. In addition, the name given to a specific
  module is purely for debugging purposes, and has no effect on event
  reporting; you can have multiple modules with the same name and different
  IDs, if you want.

Plugin Configuration Details

.. list-table::
   :header-rows: 1
   :widths: 20 80

   * - Configuration Item

     - Notes

   * - :c:macro:`RCSW_ER_PLUGIN_PRINTF`

     - Defined as ``printf()``

   * - :c:macro:`RCSW_ER_PLUGIN_INIT()`

     - Idempotent. Takes no arguments. After initialization, no modules are
       installed.

   * - :c:macro:`RCSW_ER_PLUGIN_DEINIT()`

     - Idempotent. Takes no arguments.

   * - :c:macro:`RCSW_ER_PLUGIN_REPORT()`

     - None.

   * - :c:macro:`RCSW_ER_PLUGIN_INSMOD`

     - Idempotent for the same arguments. Default reporting level after
       installation is INFO.

   * - :c:macro:`RCSW_ER_PLUGIN_LVL_CHECK`

     - Thread-safe, as long as module installation/removal is not done
       simultaneously.

   * - :c:macro:`RCSW_ER_MODNAME`

     - The name of the module. Can have any format; it can be convenient to use
       a hierarchical format such as ``foo.bar.baz`` for interoperability with
       other plugins. See also :c:macro:`RCSW_ER_MODNAME_BUILDER`.

   * - :c:macro:`RCSW_ER_MODID`

     - Must be defined uniquely for each module and/or file involved in
       logging. Well, you don't *have* to defined it uniquely, but if you don't
       messages will be emitted in surprising ways.
