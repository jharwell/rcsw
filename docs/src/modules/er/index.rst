.. _modules/er:

====================
Event Reporting (ER)
====================

The ER framework in RCSW is meant to be a flexible and adaptable "front-end" to
any number of implementations (plugins), providing a common interface and
facilities. Important aspects/terminology:

- The unit into which related functionality (and therefore event reporting) is
  grouped is called a *module*. *Modules* are file-based; that is, each ``.c``
  file can correspond to at most one module.

- A module is *active* if it has a valid *handle*; that is, if
  ``RCSW_ER_PLUGIN_INSMOD()`` was previously called for the module.

Usage
=====

To use one of the existing RCSW ER plugins in your code, you need to:

#. ``#define RCSW_ER_MODID`` and/or ``#define RCSW_ER_MODNAME`` to be the
   numeric UUID and string UUID for a module in your code and then ``#include
   <rcsw/er/client.h>``.

   .. TIP:: Not all ER plugins use both ``RCSW_ER_MODID`` and
            ``RCSW_ER_MODNAME``; see documentation for relevant plugins for
            details. Still, it's generally good practice to always define both
            to make your code more robust and compatible with any RCSW ER
            plugin.

   For example, in ``src/ds/rbuffer.c`` we have something like this near the top of
   the file::

     #define RCSW_ER_MODID 0x0000100000000000
     #define RCSW_ER_MODNAME "rcsw.ds.rbuffer"
     #include "rcsw/er/client.h"

#. Call ``RCSW_ER_MODULE_INIT()`` somewhere in your ``.c`` file, ideally in an
   initialization function. If a ``.c`` file you want to define a module for
   does not have such a function, you can put it in a common initialization
   function for your library/application. E.g.::

     void library_init(void) {
        #define RCSW_ER_MODID 0x1
        #define RCSW_ER_MODNAME "myapp.module1"
        RCSW_ER_MODULE_INIT()

        #define RCSW_ER_MODID 0x2
        #define RCSW_ER_MODNAME "myapp.module2"
        RCSW_ER_MODULE_INIT()
     }

   or::

     void library_init(void) {
        RCSW_ER_INSMOD(0x1, "myapp.module1")
        RCSW_ER_INSMOD(0x2, "myapp.module1h2")
     }

  The two forms are equivalent. The first is generally more convenient for more
  OOP-oriented modules in separate files.

.. NOTE:: If you want to be able to use the same code to define modules for
          different plugins, use :c:macro:`RCSW_ER_MODNAME_BUILDER` to define
          the names so that they will work with every plugin.

.. _er-levels:

Levels
======

The reporting levels which RCSW uses across all its plugins should be familiar
if you've dealt with logging before. All plugins support all logging levels. The
levels are, ordered from most to least severe:

- ``NONE`` - No event reporting
- ``FATAL`` - Only FATAL events are emitted
- ``ERROR`` - [FATAL, ERROR] events emitted
- ``INFO`` - [FATAL, ERROR, INFO] events emitted
- ``DEBUG`` - [FATAL, ERROR, INFO, DEBUG] events emitted
- ``TRACE`` - [FATAL, ERROR, INFO, DEBUG, TRACE] events emitted
- ``ALL`` - All events emitted

A somewhat unique feature: because RCSW works with :ref:`LIBRA
<libra:usage-capabilities>`, you can set the levels for modules in two ways:

- At compile-time, effectively disabling any events are lower severity levels;
  i.e., they will be compiled out.

- At run-time, as you would expect (if the selected plugin supports it).

Plugins
=======

.. tabs::

   .. tab:: Simple


      .. include:: plugin/simple.rst

   .. tab:: LOG4CL

      .. include:: plugin/log4cl.rst

   .. tab:: zlog

      .. include:: plugin/zlog.rst

   .. tab:: Custom

      .. include:: plugin/custom.rst


