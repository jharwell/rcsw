#
# Copyright 2023 John Harwell, All rights reserved.
#
# SPDX-License Identifier: MIT
#
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/core.cmake)

macro(rcsw_posix_configure_components)
  rcsw_core_configure_components()

  libra_component_register_as_src(
    rcsw_algorithm_SRC
    rcsw
    "${rcsw_SRC}"
    algorithm
    "src/algorithm")

  libra_component_register_as_src(
    rcsw_al_SRC
    rcsw
    "${rcsw_SRC}"
    al
    "src/al/posix")

  libra_component_register_as_src(
    rcsw_ds_SRC
    rcsw
    "${rcsw_SRC}"
    ds
    "src/ds")

  libra_component_register_as_src(
    rcsw_profile_SRC
    rcsw
    "${rcsw_SRC}"
    profile
    "src/profile")

  libra_component_register_as_src(
    rcsw_er_SRC
    rcsw
    "${rcsw_SRC}"
    er
    "src/er")

  libra_component_register_as_src(
    rcsw_utils_SRC
    rcsw
    "${rcsw_SRC}"
    utils
    "src/utils")

  set(components
    core
    algorithm
    al
    ds
    profile
    er
    utils
  )

  # If we don't do this, then we have to use the mpicc/mpicxx C/C++
  # compiler wrappers even if we are not using MPI. (Slightly) better to
  # do it this way I think.
  if (${LIBRA_MP})
    libra_component_register_as_src(
      rcsw_multiprocess_SRC
      rcsw
      "${rcsw_SRC}"
      multiprocess
      "src/multiprocess")
    set(components
      ${components}
      multiprocess
    )
  endif()

  if (${LIBRA_MT})
    libra_component_register_as_src(
      rcsw_multithread_SRC
      rcsw
      "${rcsw_SRC}"
      multithread
      "src/multithread")

    libra_component_register_as_src(
      rcsw_pulse_SRC
      rcsw
      "${rcsw_SRC}"
      pulse
      "src/pulse")

    set(components
      ${components}
      multithread
      pulse
    )
  endif()

  if (NOT ${RCSW_WITHOUT_STDIO})
    libra_component_register_as_src(
      rcsw_stdio_SRC
      rcsw
      "${rcsw_SRC}"
      stdio
      "src/stdio")
    set(components
      ${components}
      stdio
    )
  endif()

  # Root project (not used in find_package())
  if (NOT rcsw_FIND_COMPONENTS)
    set(rcsw_FIND_COMPONENTS ${components})
  endif()
endmacro()
