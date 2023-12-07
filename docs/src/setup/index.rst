.. SPDX-License-Identifier: MIT

.. _setup:

===============
Setting Up RCSW
===============


.. _setup-devel:

Developer Setup
===============

#. RCSW uses :ref:`libra:main` so go to :ref:`libra:usage-req` and install any
   needed packages.

#. Clone RCSW and init LIBRA::

     git clone git@github.com:jharwell/rcsw.git
     cd rcsw
     git submodule update --init --remote --recursive

#. Build RCSW. From the root of the repo::

     mkdir build && cd build
     cmake <ARGS> ..
     make

   ``<ARGS>`` is a list of cmake arguments.

   .. IMPORTANT:: RCSW and LIBRA output **VERY** thorough summaries of their
                  build configuration, so check them to make sure you are
                  building what you think you are.


   You can pass any option as part of ``<ARGS>`` that LIBRA supports (see
   :ref:`libra:usage-capabilities`) or that RCSW supports (see
   :ref:`setup-cmake`).

.. _setup-cmake:

RCSW ``cmake`` Configuration
----------------------------

.. NOTE:: All cmake configuration options which have a direct analogue in the
          code are prefixed with ``RCSW_CONFIG_``; those that don't (e.g.,
          ``RCSW_SUMMARY``) are prefixed with ``RCSW_``.

.. list-table::
   :widths: 15,75,10
   :header-rows: 1

   * - Variable

     - Description

     - Default

   * - ``RCSW_CONFIG_NOALLOC``

     - Is dynamic heap memory allocation disallowed ? If TRUE, implies:

       - :c:macro:`RCSW_NOALLOC_META`
       - :c:macro:`RCSW_NOALLOC_DATA`
       - :c:macro:`RCSW_NOALLOC_HANDLE`

       where applicable for all modules.

     - ``NO``  (heap memory allocation allowed).

   * - ``RCSW_CONFIG_ZALLOC``

     - Should all allocated memory (dynamically or by the application) be zeroed
       before use by RCSW ? See :c:macro:`RCSW_ZALLOC` for implications of this
       variable.

     - ``NO``

   * - ``RCSW_CONFIG_STDIO_PRINTF_BUFSIZE``

     - Internal ntoa() conversion buffer size in RCSW's printf()
       implementation. Must be big enough to hold one converted numeric number
       including padding.

     - 32

   * - ``RCSW_CONFIG_STDIO_PRINTF_WITH_DEC``

     - Compile in support for the decimal notation floating point conversion
       specifiers:

       - %f

       - %F

     - ``YES``


   * - ``RCSW_CONFIG_STDIO_PRINTF_WITH_EXP``

     - Compile in support for the exponential notation floating point conversion
       specifiers:

       - %e
       - %g
       - %E
       - %G

     - ``YES``

   * - ``RCSW_CONFIG_STDIO_PRINTF_WITH_WRITEBACK``

     - Compile in support for the length write-back specifier (%n).

     - ``YES``

   * - ``RCSW_CONFIG_STDIO_PRINTF_DEFAULT_FLOAT_PREC``

     - Set default precision for the floating point conversion specifiers (the C
       standard sets this at 6).

     - 6

   * - ``RCSW_CONFIG_STDIO_PRINTF_EXP_DIGIT_THRESH``

     - According to the C standard, printf() must be able to print any integral
       number in floating-point notation, regardless of length, when using %f,
       which can overflow buffers. To be safe, values taking more than this many
       digits to print are switched to exponential notation.

     - 9

   * - ``RCSW_CONFIG_STDIO_PRINTF_WITH_LL``

     - Support for the long long integral types (with the ll, z and t length
       modifiers) for specifiers:

       - %d
       - %i
       - %o
       - %x
       - %X
       - %u
       - %p

       'L' (long double) is not supported.

     - ``YES``

   * - ``RCSW_CONFIG_STDIO_MATH_LOG10_TERMS``

     - The number of terms in a Taylor series expansion of log_10(x) to use for
       approximation - including the power-zero term (i.e. the value at the
       point of expansion). Can be [1,2,3,4].

     - 4

   * - ``RCSW_CONFIG_STDIO_MATH_LOG10_CHECK_NULL``

     - Be extra-safe, and don't assume format specifiers are completed correctly
       before the format string end. Requires runtime checking.

     - ``YES``

   * - ``RCSW_CONFIG_STDIO_PUTCHAR``

     - The name of the putchar()-like function that RCSW's printf()
       implementation should link with to write characters to stdout. Must have
       same signature as putchar().

     - ``putchar`` (assume linking with stdlibs OR that this is the name of your
       custom function).

   * - ``RCSW_CONFIG_STDIO_GETCHAR``

     - The name of the getchar()-like function that RCSW's printf()
       implementation should link with to read characters from stdin. Must have
       same signature as getchar().

     - ``getchar`` (assume linking with stdlibs OR that this is the name of your
       custom function).

   * - ``RCSW_CONFIG_NO_GRIND``

     - Compile out:

       - :c:macro:`RCSW_GRIND_START()`
       - :c:macro:`RCSW_GRIND_END()`
       - :c:macro:`RCSW_GRIND_COUNT()`
       - :c:macro:`RCSW_GRIND_TICK()`

       This variable enables you to leave in timing collection in application
       code, and only collect it when you need to.

     - ``NO``

   * - ``RCSW_CONFIG_ER_PLUGIN``

     - The default event reporting plugin to use. See :ref:`modules/er` for
       details.

     - ``LOG4CL``

   * - ``RCSW_CONFIG_ER_PLUGIN_PATH``

     - The path to the ``#include`` file for a custom ER plugin. See
       :ref:`modules/er` for details.

     - Empty

   * - ``RCSW_CONFIG_PTR_ALIGN``

     - Override the pointer alignment used to store references to all
       application data which RCSW manages.

       Some architectures can handle trying to use a ``uint32_t*`` to access
       something where ``addr % (sizeof(uint32_t)) > 0``, and on others doing so
       will cause a hardware trap.  Generally you want to use higher alignments
       to store data, as that results in better cache line usage, but this is
       not always possible. RCSW tries to detect the best value for alignment,
       but if it is being built for a novel architecture it will fall back on
       storing everything internally using byte pointers for safety. You can
       override this if you know better.

       Must be [1,2,4].

     - - x86 - 4
       - ARM - 1
       - Everything else - 1

   * - ``RCSW_SUMMARY``

     - Show a summary of all RCSW-specific variables when running ``cmake``.

     - ``YES``

   * - ``RCSW_CONFIG_LIBTYPE``

     - What type of library to build RCSW as (SHARED or STATIC).

     - ``STATIC``

   * - ``RCSW_BUILD_FOR``

     - Meta-option defining what "platform" RCSW should build against/for. Valid
       values:

       - ``POSIX`` - Build for linux/POSIX-y OSes. This is the most
         full-featured RCSW.

       - ``BAREMETAL`` - Build for baremetal environments without an OS. Still
         enables/uses stdlib from the selected compiler. If you don't want/need
         stdlib, then you can also pass ``LIBRA_NOSTDLIB``.


     - ``POSIX``

   * - ``RCSW_WITHOUT_STDIO``

     - Meta-option to remove the STDIO module from compilation. Useful if you
       don't need it to reduce compilation times.

     - ``NO``
