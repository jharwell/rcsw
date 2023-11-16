#
# Copyright 2023 John Harwell, All rights reserved.
#
# SPDX-License Identifier: MIT
#
macro(rcsw_core_configure_components)
  string(CONCAT core_regex
    "src/common"
  )

  libra_component_register_as_src(
    rcsw_core_SRC
    rcsw
    "${rcsw_SRC}"
    core
    "${core_regex}")

endmacro()
