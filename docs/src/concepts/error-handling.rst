.. SPDX-License-Identifier: MIT

.. _concepts/error-handling:

==============
Error Handling
==============

RCSW functions report failure through their return value and, where it helps,
``errno``. There are no exceptions, callbacks or global error state beyond
``errno``.

Return values
=============

- Functions that can fail and return nothing else return :c:enum:`status_t`:
  ``OK`` (0) or ``ERROR`` (-1).
- Functions that return a pointer return ``NULL`` on failure.
- Functions that return an index return -1 on failure.
- Query functions (``xx_size()``, ``xx_isempty()``, ...) return 0, ``false``
  or ``NULL`` when given an invalid handle. These values can't be told apart
  from a valid answer, so validate handles before relying on them.

``errno`` is set on many failure paths, but not all of them; treat it as a
hint about why something failed, and the return value as the fact that it
did. :ref:`library/ds/error-codes` lists the values the data structures use.

.. _concepts/error-handling/fpc:

Function preconditions
======================

Public functions check their arguments with the FPC (function precondition)
macros in ``rcsw/core/fpc.h``. What a failed check does is chosen at build
time with ``LIBRA_FPC``:

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - Mode
     - On a failed precondition

   * - ``RETURN`` (default)
     - The function sets ``errno = EINVAL`` and returns its error value
       (``ERROR``, ``NULL``, -1, 0 or ``false``, per the rules above).

   * - ``ABORT``
     - The check is an ``assert()``. In a build with ``NDEBUG`` defined,
       nothing is checked.

   * - ``NONE``
     - Nothing is checked. Invalid arguments are undefined behaviour.

The checks are short-circuited: the first failed condition decides the result.

.. _concepts/error-handling/cleanup:

Cleanup on failure
==================

Functions that acquire resources step by step use a single ``error:`` label
for cleanup. The checking macros jump to it:

.. list-table::
   :header-rows: 1
   :widths: 35 65

   * - Macro
     - Jumps to ``error`` when

   * - :c:macro:`RCSW_CHECK` ``(cond)``
     - ``cond`` is false.

   * - :c:macro:`RCSW_CHECK_PTR` ``(ptr)``
     - ``ptr`` is ``NULL``.

   * - :c:macro:`RCSW_CHECK_FD` ``(fd)``
     - ``fd`` is negative.

   * - :c:macro:`ER_CHECK` ``(cond, msg, ...)``
     - ``cond`` is false, after reporting ``msg`` at ERROR level.

   * - :c:macro:`ER_SENTINEL` ``(msg, ...)``
     - Always, after reporting ``msg`` at ERROR level. Marks code that should
       be unreachable.

.. code-block:: c

   status_t widget_init(struct widget* w) {
     w->buf = malloc(64);
     RCSW_CHECK_PTR(w->buf);
     ER_CHECK(OK == bus_attach(w), "Failed to attach widget");
     return OK;

   error:
     free(w->buf);
     return ERROR;
   }

For conditions that must never fail, :c:macro:`ER_ASSERT` reports a FATAL
event and then calls ``assert()``, and :c:macro:`ER_FATAL_SENTINEL` reports a
FATAL event and calls ``abort()``. With ``NDEBUG`` defined, ``ER_ASSERT``
still evaluates the condition and reports the event, but the ``assert()``
doesn't stop the program.
