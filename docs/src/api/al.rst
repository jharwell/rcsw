.. SPDX-License-Identifier: MIT

.. _api/al:

==========================
Abstraction Layer (``al``)
==========================

.. doxygengroup:: al
   :desc-only:

``rcsw/al/al.h``
----------------

.. doxygenfile:: al.h

``rcsw/al/clock.h``
-------------------

Includes the clock interface for the platform RCSW is built for:
``rcsw/al/posix/clock.h`` or ``rcsw/al/baremetal/clock.h``. Both declare the
same functions.

.. Three headers share the name clock.h, which doxygenfile can't tell apart,
   so the functions are listed individually.

.. doxygenfunction:: clock_monotime

.. doxygenfunction:: clock_realtime

``rcsw/al/types.h``
-------------------

.. doxygenfile:: types.h
