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
    rcsw_tool_SRC
    rcsw
    "${rcsw_SRC}"
    tool
    "src/tool/minimon")

  set(components
    core
    al
    tool
  )


  if (NOT ${LIBRA_NOSTDLIB})
    libra_component_register_as_src(
      rcsw_er_SRC
      rcsw
      "${rcsw_SRC}"
      er
      "src/er")

    libra_component_register_as_src(
      rcsw_ds_SRC
      rcsw
      "${rcsw_SRC}"
      ds
      "src/ds")
    string(CONCAT utils_regex
      "src/utils/mem|"
        "src/utils/hash|"
        "src/utils/checksum"
      )

    libra_component_register_as_src(
      rcsw_utils_SRC
      rcsw
      "${rcsw_SRC}"
      util
      "${utils_regex}")


    set(components
      ${components}
      er
      ds
      utils
    )

  else()
    libra_component_register_as_src(
      rcsw_utils_SRC
      rcsw
      "${rcsw_SRC}"
      utils
      "src/utils")

    set(components
      ${components}
      utils
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
