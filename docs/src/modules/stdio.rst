.. _modules/stdio:

==============================
Simple I/O Replacement Library
==============================

A re-implementation of the libc routines needed to make printf() work. Mostly
stolen from `<https://github.com/eyalroz/printf>`_. For debugging in
bare-metal/embedded environments when libc is not available (e.g., bootstraps).
