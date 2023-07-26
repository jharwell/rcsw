The simple bare-bones logger.

This plugin uses RCSW's built-in :c:func:`stdio_printf()` and minimal stdlib
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

Where :c:macro:`RCSW_ER_MODNAME` is the #define defining the name of the
module. If it is not defined, ``__FILE_NAME__`` is used. ``LVL`` is one of:
[FATAL, ERROR, INFO, WARN, DEBUG, TRACE].

This plugin is useful in:

- Bare metal environments such as bootstraps without an OS.

- Bare metal hardware validation tests.
