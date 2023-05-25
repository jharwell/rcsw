.. SPDX-License-Identifier:  MIT

==========================
Reusable C Software (RCSW)
==========================

.. |ci-master| image:: https://github.com/jharwell/rcsw/actions/workflows/ci.yml/badge.svg?branch=master

.. |ci-devel| image:: https://github.com/jharwell/rcsw/actions/workflows/ci.yml/badge.svg?branch=devel

.. |license| image:: https://img.shields.io/github/license/jharwell/rcsw
                     :target: https://img.shields.io/github/license/jharwell/rcsw

.. |docs| image:: https://github.com/jharwell/rcsw/actions/workflows/pages.yml/badge.svg?branch=master
                  :target: https://jharwell.github.io/rcsw

.. |maintenance| image:: https://img.shields.io/badge/Maintained%3F-yes-green.svg

.. |coverage-master| image:: https://coveralls.io/repos/github/jharwell/rcsw/badge.svg?branch=master
                             :target: https://coveralls.io/github/jharwell/rcsw?branch=master

.. |coverage-devel| image:: https://coveralls.io/repos/github/jharwell/rcsw/badge.svg?branch=devel
                             :target: https://coveralls.io/github/jharwell/rcsw?branch=devel


:Release:

   |ci-master| |coverage-master|

:Development:

   |ci-devel| |coverage-devel|

:Misc:

   |license| |docs| |maintenance|


RCSW is a collection of reusable (but not necessarily generic) C software
modules, in the style of the C++ STL. Briefly, it contains:

- Data structures: ringbuffer, linked list, FIFIO, dynamic array, binary search
  tree, red-black tree, order statistics tree, hashmap, binary heap, etc.

- Algorithms: binary search, quirksort, mergesort, insertion sort, radix sort,
  etc.

- Multithreading: memory pool, binary semaphore, mutex, producer-consumer queue,
  reader/writer lock, etc.

- Simple I/O: re-implementation of the libc routines needed to make printf()
  work; very useful in base-metal environments (e.g., bootstraps)

- Utils: XOR checksums, hashing functions, bit masking, memory manipulation
  (e.g., read-modify-write with verify)

Full project documentation is here: `<https://jharwell.github.io/rcsw>`_.

License
=======

This project is licensed under the MIT license.
