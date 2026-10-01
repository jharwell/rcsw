# cmake-format: off
# ##############################################################################
# rcsw project configuration (included by LIBRA)
#
# Split into modules, included in dependency order:
#
# packages.cmake    third-party packages (Catch2, printf)
# build.cmake       rcsw_message(), user-facing options, platform settings
# components.cmake  component options and what each component needs
# targets.cmake     libraries, headers, definitions, includes, links
# install.cmake     install, exports, packaging
# summary.cmake     configuration summary
# ##############################################################################
# cmake-format: on
include(${CMAKE_CURRENT_LIST_DIR}/packages.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/build.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/components.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/targets.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/install.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/summary.cmake)
