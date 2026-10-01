.. SPDX-License-Identifier: MIT

.. _library/console:

=======
Console
=======

Interactive tools for bootloader and bootstrap development on bare-metal
targets.

Minimon
=======

``#include "rcsw/console/minimon.h"``

:c:struct:`minimon` is a small, extensible interactive monitor for bare-metal
environments. It is designed for initial board bring-up and hardware
validation — scenarios where you need to inspect memory, load firmware, or
transfer control without a full debugger attached.

Minimon has no dynamic memory requirements and no OS dependencies, making it
usable before an RTOS or heap allocator is initialized. It communicates over a
serial stream through :c:func:`stdio_putchar` and :c:func:`stdio_getchar`,
i.e. the configured :cmake:variable:`RCSW_CONFIG_STDIO_PUTCHAR` and
:cmake:variable:`RCSW_CONFIG_STDIO_GETCHAR` functions, so it needs the stdio
component (see :ref:`concepts/components`). An optional second stream,
:c:member:`minimon_config.stream1`, carries data for the ``load`` and ``send``
commands so it doesn't mix with the interactive session.

Built-in commands cover reading and writing memory words (``read``,
``write``), jumping to an address (``jump``), loading data from the input
stream (``load``), and sending a memory range out over a stream (``send``, in
raw, NMEA-framed, or text/hex-dump format). ``help`` is always present. See
:c:struct:`minimon_cmd` and the constant ``MINIMON_CMD_MAX_ARGS`` in the header
for the full command interface and limits.

Custom commands are registered through :c:struct:`minimon_config`, passed to
:c:func:`minimon_init`; they can extend or replace the built-ins. The command
table is used in place, not copied, so it must outlive the monitor; a
``static const`` table is the usual choice. Then
:c:func:`minimon_start` runs the monitor loop and does not return. See
:c:struct:`minimon` for the registration workflow and :c:struct:`minimon_cmd`
for the field reference.
