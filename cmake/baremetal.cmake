#
# Copyright 2023 John Harwell, All rights reserved.
#
# SPDX-License Identifier: MIT
#
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/core.cmake)

macro(rcsw_baremetal_configure_components)
  rcsw_core_configure_components()

  libra_component_register_as_src(
    rcsw_al_SRC
    rcsw
    "${rcsw_SRC}"
    al
    "src/al/baremetal")

  libra_component_register_as_src(
    rcsw_ds_SRC
    rcsw
    "${rcsw_SRC}"
    ds
    "src/ds")

  libra_component_register_as_src(
    rcsw_utils_SRC
    rcsw
    "${rcsw_SRC}"
    util
    "src/utils")

  libra_component_register_as_src(
    rcsw_er_SRC
    rcsw
    "${rcsw_SRC}"
    er
    "src/er")

  set(components
    core
    al
    ds
    er
    utils
  )

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
