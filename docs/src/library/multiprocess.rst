.. _library/multiprocess:

============
Multiprocess
============

A collection of modules for multi-process applications. All MPI routines use
**standard MPI** (``<mpi.h>``); OpenMPI and MPICH have been tested. RCSW does
not provide an MPI implementation or wrapper library—your build environment
must supply a compatible MPI installation accessible under
``CMAKE_INSTALL_PREFIX``.

.. NOTE::

   Multiprocess support is only enabled when ``RCSW_BUILD_FOR=POSIX``. It is
   not available in baremetal builds.

Process Management
==================

:c:struct:`procm` provides utilities for managing multi-process applications,
including process spawning and monitoring helpers.

MPI Algorithms
==============

The following parallel algorithms are provided as a limited subset of
MPI-based computation. They require all participating processes to call the
relevant init function before use, as they rely on collective MPI operations
(``MPI_Allreduce``, ``MPI_Scatter``, ``MPI_Gather``, etc.).

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Module
     - Description

   * - :c:struct:`mpi_radix_sorter`
     - Distributed radix sort. Data is partitioned across ``mpi_world_size``
       processes; each process sorts its chunk locally, then a global
       redistribution pass is performed per digit. The number of elements
       must be evenly divisible by ``mpi_world_size``. Initialize with
       :c:func:`mpi_radix_sorter_init()` (all ranks); call
       :c:func:`mpi_radix_sorter_run()` to execute. Rank 0 holds the
       sorted result on return.

   * - :c:struct:`mpi_spmv_mult`
     - Distributed sparse matrix-vector multiplication using CSR-format
       matrices. Data distribution and result gathering are handled
       internally via MPI collectives.

Error Handling
==============

Functions return ``NULL`` or ``ERROR`` on failure. MPI errors surface as
``MPI_ERR_*`` codes via the default MPI error handler; RCSW does not
intercept MPI error returns beyond precondition checks.
