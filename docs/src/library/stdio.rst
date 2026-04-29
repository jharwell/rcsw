.. _library/stdio:

==============================
Simple I/O Replacement Library
==============================

``#include "rcsw/stdio/printf.h"``
``#include "rcsw/stdio/stdio.h"``

RCSW's stdio module provides a printf-family implementation and basic I/O
helpers for environments where libc is not available. It wraps
`eyalroz/printf <https://github.com/eyalroz/printf>`_ with the RCSW
naming convention and platform abstraction layer.

When to Use This Module
=======================

Use the stdio module when:

- You are building for a bare-metal target without libc (e.g., a
  bootstrap, bootloader, or hardware validation test).
- You are using the ``simple`` ER plugin, which requires
  :c:func:`stdio_printf()` as its output function.
- You need printf output routed through a custom BSP character output
  function rather than the system's ``putchar``.

On POSIX targets where libc is available, you generally do not need this
module. Disable it at build time with :cmake:variable:`RCSW_CONFIG_NO_STDIO`
``=YES`` if you are not using it.

Platform Abstraction
====================

The stdio module routes character I/O through two CMake-configurable
hooks:

- :cmake:variable:`RCSW_CONFIG_STDIO_PUTCHAR`
- :cmake:variable:`RCSW_CONFIG_STDIO_GETCHAR`

These macros name functions that **you must provide** in your platform
layer. RCSW declares them but does not implement them beyond routing
through the configured name.

printf Formatting
=================

Float support, long long support, buffer sizes, and precision defaults are
controlled by `eyalroz/printf <https://github.com/eyalroz/printf>`_ CMake
options. To override an option, set it in your CMake invocation before
including RCSW — CPM respects already-set cache variables.

String and Integer Utilities
============================

``#include "rcsw/stdio/stdio.h"``

Bare-metal replacements for common libc I/O and conversion functions.
See the header for full signatures. Highlights:

- :c:func:`stdio_puts()` writes a string without a trailing newline
  (unlike POSIX ``puts``).
- :c:func:`stdio_atoi()` requires an explicit base argument and a ``0x``
  prefix for hex input.
- :c:func:`stdio_itoad()` and :c:func:`stdio_itoax()` are 32-bit only.
