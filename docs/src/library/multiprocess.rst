.. SPDX-License-Identifier: MIT

.. _library/multiprocess:

============
Multiprocess
============

``#include "rcsw/multiprocess/procm.h"``

Process management helpers for POSIX systems. Not available in bare-metal
builds.

- :c:func:`procm_socket_lock` restricts the calling process to the CPUs of one
  socket. Linux only: it reads the CPU topology from sysfs.
- :c:func:`procm_fork_exec` forks and ``execv()``\ s a command (no ``PATH``
  search), optionally changing the child's working directory, silencing its
  stdout, and connecting a pipe to its stdin.

Error Handling
==============

:c:func:`procm_socket_lock` returns ``ERROR`` on failure; see
:ref:`concepts/error-handling`. :c:func:`procm_fork_exec` returns the child's
process ID to the parent, or -1 if ``fork()`` failed. A child that can't set
up or ``exec`` writes a message to stderr and exits with ``EXIT_FAILURE``.
