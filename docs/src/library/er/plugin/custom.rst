.. SPDX-License-Identifier: MIT

A custom plugin which is defined exactly how you want in terms of modules,
levels, etc. To integrate your plugin with RCSW, you must create a ``.h`` file
with a few ``#define`` macros. If your plugin uses :c:macro:`RCSW_ER_MODID`, it
should support 64-bit IDs if you want to use RCSW with it.

.. IMPORTANT:: Your ``.h`` file is **NOT** installed/packaged with RCSW, so you
               will need to ensure that it is findable by any applications you
               want to use with RCSW+your custom ER plugin.

Your file must define every macro below; RCSW stops the build with an
``#error`` if one is missing. A macro your plugin has no use for can be
defined as nothing (or, for ``RCSW_ER_PLUGIN_LVL_CHECK()``, as a true value).
You can of course have whatever else you want in the file.

.. tab-set::

   .. tab-item:: ``RCSW_ER_PLUGIN_REPORT()``

      The main ER plugin hook. Will be called as part of every
      :c:macro:`ER_WARN()`, etc. statement. Arguments:

      - ``LVL`` - The level of the statement, as a bare token (``WARN``,
        ``INFO``, ...). See :ref:`concepts/event-reporting/levels` for details.

      - ``HANDLE`` - Whatever was returned from ``RCSW_ER_PLUGIN_HANDLE()``.

      - ``ID`` - The ID of the current module (file): :c:macro:`RCSW_ER_MODID`,
        or ``0xFFFFFFFF`` if the file doesn't define one.

      - ``NAME`` - The name of the current module (file)

      - ``MSG`` - The message string, with ``"\r\n"`` already appended

      - ``...`` - Any additional arguments for the message string

      .. NOTE:: This macro is used as a statement inside RCSW's ER machinery,
                so it must expand to a complete statement: a braced block, or
                a call ending in ``;``.

   .. tab-item:: ``RCSW_ER_PLUGIN_PRINTF``

       The name of a ``printf()``-like function with the same signature; used to
       define the :c:macro:`PRINTF()` / :c:macro:`DPRINTF()` macros. If you
       define it as nothing, you can't use those macros.

   .. tab-item:: ``RCSW_ER_PLUGIN_INIT()``

       A framework initialization hook, called by ``RCSW_ER_INIT()``; should be
       idempotent. Can take any number of arguments of any type. If it is not
       needed by your plugin, ``#define`` as nothing.

   .. tab-item:: ``RCSW_ER_PLUGIN_DEINIT()``

      A framework shutdown hook, called by ``RCSW_ER_DEINIT()``; should be
      idempotent. Can take any number of arguments of any type. If it is not
      needed by your plugin ``#define`` as nothing.

   .. tab-item:: ``RCSW_ER_PLUGIN_INSMOD()``

      Arguments:

      - ``ID`` - The numeric ID for the module.

      - ``NAME`` - The string name for the module.

      Install/enable a module with the specified ID and name. If not needed by
      your plugin, ``#define`` as nothing.

   .. tab-item:: ``RCSW_ER_PLUGIN_HANDLE()``

      Arguments:

      - ``ID`` - The numeric ID for the module.

      - ``NAME`` - The string name for the module.

      Get a logger "handle" of some kind which contains the necessary
      information to determine if a given module is enabled. For example, in the
      LOG4CL plugin, the :c:func:`log4cl_mod_query()` function serves this
      purpose.

      RCSW doesn't inspect the handle; it only passes it to
      ``RCSW_ER_PLUGIN_LVL_CHECK()``. If the module with the specified
      ``ID, NAME`` is not enabled, return something (such as ``NULL``) that
      your level check treats as "don't emit".

      If not needed by your plugin, ``#define`` as nothing.

   .. tab-item:: ``RCSW_ER_PLUGIN_LVL_CHECK()``

      Arguments:

      - ``HANDLE`` - The module handle returned by ``RCSW_ER_PLUGIN_HANDLE()``.

      - ``LVL`` - The level associated with the current reporting statement,
        as a bare token.

      Given an active module ``HANDLE``, determine if the statement with the
      specified ``LVL`` should be emitted or not.

      If not needed by your plugin, ``#define`` as a truth-y value,
      such as 1.

   .. tab-item:: ``RCSW_ER_PLUGIN_MODNAME_COMPONENT_SEPARATOR``

      The string :c:macro:`RCSW_ER_MODNAME_BUILDER` puts between name
      components, such as ``"."``.
