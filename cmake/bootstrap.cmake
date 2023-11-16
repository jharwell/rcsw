#
# Copyright 2023 John Harwell, All rights reserved.
#
# SPDX-License Identifier: MIT
#
macro(rcsw_bootstrap_configure_components)
  libra_component_register_as_src(
    rcsw_al_SRC
    rcsw
    "${rcsw_SRC}"
    al
    "src/al/bootstrap")

  string(CONCAT utils_regex
    "src/utils/mem|"
    "src/utils/hash|"
    "src/utils/checksum"
  )
  libra_component_register_as_src(
    rcsw_utils_SRC
    rcsw
    "${rcsw_SRC}"
    utils
    "${utils_regex}")

  set(components
    al
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
