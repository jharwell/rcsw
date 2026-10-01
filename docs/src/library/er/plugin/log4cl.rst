.. SPDX-License-Identifier: MIT

A lighter/simpler version of `log4c <https://log4c.sourceforge.net/>`_.

In this plugin, each source file within RCSW and of each project which links
with RCSW can define a logging "module"; modules are file-based, and therefore
you can't have multiple modules/loggers in a single file. If you architect your
projects well, this should not be a burdensome restriction. You *can* split a
logging module across several files by defining the same
:c:macro:`RCSW_ER_MODID` in each; LOG4CL identifies modules by ID, not name.

Each module can be enabled/disabled independently in a lightweight manner;
unlike log4j loggers, the enable/disable status of one logger/module does not
have any effect on the status of another. That is, the hierarchy is "flat".

This plugin does not provide most features found in log4c, with the exception of
levels: FATAL, ERROR, WARN, INFO, DEBUG, TRACE. If you want something with features
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

.. rubric:: Plugin configuration

.. list-table::
   :header-rows: 1
   :widths: 20 80

   * - Configuration Item

     - Notes

   * - ``RCSW_ER_PLUGIN_PRINTF``

     - Defined as ``printf()``

   * - ``RCSW_ER_PLUGIN_INIT()``

     - Idempotent. Takes no arguments. After initialization, no modules are
       installed.

   * - ``RCSW_ER_PLUGIN_DEINIT()``

     - Idempotent. Takes no arguments.

   * - ``RCSW_ER_PLUGIN_REPORT()``

     - None.

   * - ``RCSW_ER_PLUGIN_INSMOD``

     - Idempotent for the same arguments. Default reporting level after
       installation is INFO.

   * - ``RCSW_ER_PLUGIN_LVL_CHECK``

     - Thread-safe. On POSIX, installation, removal and lookup are serialized
       internally; don't call ``RCSW_ER_DEINIT()`` while other threads are
       reporting.

   * - :c:macro:`RCSW_ER_MODNAME`

     - The name of the module. Can have any format; it can be convenient to use
       a hierarchical format such as ``foo.bar.baz`` for interoperability with
       other plugins. See also :c:macro:`RCSW_ER_MODNAME_BUILDER`.

   * - :c:macro:`RCSW_ER_MODID`

     - Identifies the module, so give each module its own ID (see
       :ref:`concepts/event-reporting/ids`). Files that don't define one all
       get ``0xFFFFFFFF`` and therefore share a single module.
