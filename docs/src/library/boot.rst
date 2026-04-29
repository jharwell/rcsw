.. _library/boot:

====
Boot
====

A collection of functionality useful during bootloader and bootstrap
development on bare-metal targets. Not available on POSIX builds.

Minimon
=======

``#include "rcsw/tool/minimon.h"``

:c:struct:`minimon` is a small, extensible interactive monitor for bare-metal
environments. It is designed for initial board bring-up and hardware
validation — scenarios where you need to inspect memory, load firmware, or
transfer control without a full debugger attached.

Minimon has no dynamic memory requirements and no OS dependencies, making it
usable before an RTOS or heap allocator is initialized. It communicates over a
serial stream using the configured :cmake:variable:`RCSW_CONFIG_STDIO_PUTCHAR`
and :cmake:variable:`RCSW_CONFIG_STDIO_GETCHAR` hooks.

Built-in commands cover reading and writing memory words, jumping to an
address, loading data from the input stream, and sending a memory range
out over a stream (in raw, NMEA-framed, or text/hex-dump format). See
:c:struct:`minimon_cmd` and the constants ``MINIMON_CMD_MAX_ARGS`` and
``MINIMON_MAX_CMDS`` in the header for the full command interface and
limits.

Custom commands are registered via :c:struct:`minimon_params` — see the
header-level documentation in ``minimon.h`` for the registration workflow
and the :c:struct:`minimon_cmd` field reference.
