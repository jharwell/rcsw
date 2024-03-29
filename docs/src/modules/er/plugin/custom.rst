A custom plugin which is defined exactly how you want in terms of modules,
levels, etc. To integrate your plugin with RCSW, you must create a ``.h`` file
with a few ``#define`` macros. Your ``.h`` file must be specified at configure
time via ``-DRCSW_CONFIG_ER_PLUGIN_PATH=/path/to/your/file``.

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
       used to define the :c:macro:`PRINTF()` / :c:macro:`DPRINTF()` macros.

     - No; if omitted, you can't use the :c:macro:`PRINTF()` /
       :c:macro:`DPRINTF()` macros.

     - ``#define`` as nothing

   * - ``RCSW_ER_PLUGIN_INIT(...)``

     - A framework initialization hook which RCSW will call in its internal
       modules; should be idempotent. Can take any number of arguments of any
       type.

     - ``#define`` as nothing.

   * - ``RCSW_ER_PLUGIN_DEINIT()``

     - A framework shutdown hook; should be idempotent. Should not take any
       arguments. Not used by RCSW currently.

     - No

     - ``#define`` as nothing.

   * - ``RCSW_ER_PLUGIN_REPORT(LVL, HANDLE, ID, NAME, MSG, ...)``

     - The main ER plugin hook. Will be called as part of every
       :c:macro:`ER_WARN()`, etc. statement. Arguments:

       - ``LVL`` - The level of the statement. See :ref:`er-levels` for
         details.

       - ``HANDLE`` - Whatever was returned from ``RCSW_ER_PLUGIN_HANDLE()``.

       - ``ID`` - The ID of the current module (file). This will expand to
         nothing if ``RCSW_ER_MODID`` is not defined.

       - ``NAME`` - The name of the current module (file)

       - ``MSG`` - The message string

       - ``...`` - Any additional arguments for the message string

     .. NOTE:: Because this macro is used as a statement inside RCSW's ER
               machinery, it must end in ``;``.

     - Yes

     - N/A

   * - ``RCSW_ER_PLUGIN_INSMOD(ID, NAME)``

     - Install/enable a module with the specified ID and name.

     - No

     - ``#define`` as nothing.

   * - ``RCSW_ER_PLUGIN_HANDLE(ID, NAME)``

     - Get a logger "handle" of some kind which contains the necessary
       information to determine if a given module is enabled. For example, in
       the SIMPLE plugin, the :c:func:`log4cl_mod_query()` function serves this
       purpose.

       If the module with the specified ``ID, NAME`` is not enabled, then the
       handle should be a false-y value, like 0 or NULL.

     - No

     - ``#define`` as nothing.

   * - ``RCSW_ER_PLUGIN_LVL_CHECK(HANDLE, LVL)``

     - Given an active module ``HANDLE``, determine if the statement with the
       specified ``LVL`` should be emitted or not.

     - No

     - ``#define`` as a truth-y value, such as 1.

A few important notes:

- If your plugin uses :c:macro:`RCSW_ER_MODID`, it should support 64-bit IDs if
  you want to use RCSW with it.
