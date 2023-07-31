A lighter/simpler version of `log4c <https://log4c.sourceforge.net/>`_.

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

Where :c:macro:`RCSW_ER_MODNAME` is the #define defining the name of the
module. If it is not defined, ``__FILE_NAME__`` is used. ``LVL`` is one of
[FATAL, ERROR, INFO, WARN, DEBUG, TRACE].

This is useful in:

- Medium-complexity embedded systems with limited resources.

- Systems where you want to have multiple modules, only some of which should
  be enabled, but don't need the hierarchical logging of log4c. In this
  scheme, modules are either enabled or not whether a given module is enabled
  has no effect on other modules. In addition, the name given to a specific
  module is purely for debugging purposes, and has no effect on event
  reporting; you can have multiple modules with the same name and different
  IDs, if you want.
