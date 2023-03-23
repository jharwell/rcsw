.. SPDX-License-Identifier: MIT

.. _ln-rcsw-setup:

===============
Setting Up RCSW
===============

.. _ln-rcsw-setup-user:

User Setup
==========

#. Install RCSW package(s). Download your chosen ``.deb``, ``.rpm`` or
   ``.tar.gz``, according to your preference.

.. _ln-rcsw-setup-devel:

Developer Setup
===============

#. RCSW uses `LIBRA <https://libra2.readthedocs.io>`_ so go to
   :ref:`ln-libra-req` and install any needed packages.


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
   :ref:`ln-libra-capabilities`).
