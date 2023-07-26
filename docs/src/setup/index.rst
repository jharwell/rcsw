.. SPDX-License-Identifier: MIT

.. _ln-rcsw-setup:

===============
Setting Up RCSW
===============


.. _ln-rcsw-setup-devel:

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
   :ref:`libra:usage-capabilities`).
