################################################################################
# Configuration Options
################################################################################
# We might be linking with a shared library
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(rcsw_CHECK_LANGUAGE "C")

set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 2)
set(PROJECT_VERSION_PATCH 4)
set(rcsw_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

libra_configure_version(
  ${CMAKE_CURRENT_SOURCE_DIR}/src/version/version.c.in
  ${CMAKE_CURRENT_BINARY_DIR}/src/version/version.c
  rcsw_components_SRC
  )

if (NOT RCSW_STDIO_PRINTF_BUFSIZE)
  set(RCSW_STDIO_PRINTF_BUFSIZE 256)
endif()

if (NOT RCSW_STDIO_PRINTF_ADD_CR)
  set(RCSW_STDIO_PRINTF_ADD_CR YES)
endif()

################################################################################
# Components
################################################################################
libra_component_register_as_src(
  rcsw_adapter_SRC
  rcsw
  "${rcsw_SRC}"
  adapter
  "src/adapter")

libra_component_register_as_src(
  rcsw_algorithm_SRC
  rcsw
  "${rcsw_SRC}"
  algorithm
  "src/algorithm")
libra_component_register_as_src(
  rcsw_common_SRC
  rcsw
  "${rcsw_SRC}"
  common
  "src/common")
libra_component_register_as_src(
  rcsw_ds_SRC
  rcsw
  "${rcsw_SRC}"
  ds
  "src/ds")

libra_component_register_as_src(
  rcsw_mp_common_SRC
  rcsw
  "${rcsw_SRC}"
  mp_common
  "src/multiprocess/procm")
  # If we don't do this, then we have to use the mpicc/mpicxx C/C++
  # compiler wrappers even if we are not using MPI. (Slightly) better to
  # do it this way I think.
if (LIBRA_MPI)
  libra_component_register_as_src(
    rcsw_mp_mpi_SRC
    rcsw
    "${rcsw_SRC}"
    mp_mpi
    "src/multiprocess/mpi")
endif()
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
libra_component_register_as_src(
  rcsw_stdio_SRC
  rcsw
  "${rcsw_SRC}"
  stdio
  "src/stdio")
libra_component_register_as_src(
  rcsw_utils_SRC
  rcsw
  "${rcsw_SRC}"
  utils
  "src/utils")

# Root project (not used in find_package())
if (NOT rcsw_FIND_COMPONENTS)
  set(rcsw_FIND_COMPONENTS
    common
    algorithm
    ds
    adapter
    multithread
    mp_common
    pulse
    stdio
    utils
    )
endif()

libra_requested_components_check(rcsw)

################################################################################
# Libraries
################################################################################
# Create the source for the SINGLE library to build by combining the
# source of the selected components
foreach(component ${rcsw_FIND_COMPONENTS})
  if(${rcsw_${component}_FOUND})
    list(APPEND rcsw_components_SRC ${rcsw_} ${rcsw_${component}_SRC})
  endif()
endforeach()

set(rcsw_LIBRARY rcsw)
add_library(
  ${rcsw_LIBRARY}
  STATIC
  ${rcsw_components_SRC}
  )

set(rcsw_LIBRARY_NAME rcsw)
set_target_properties(${rcsw_LIBRARY}
  PROPERTIES
  OUTPUT_NAME ${rcsw_LIBRARY_NAME}
  )

# Setting this results in TWO files being installed: the actual
# library with the version embedded, and a symlink to the actual
# library with the same name sans the embedded version (if rcsw is
# built as a shared library).
set_target_properties(${rcsw_LIBRARY}
  PROPERTIES
  VERSION ${rcsw_VERSION}
  SOVERSION ${rcsw_VERSION}
  )

########################################
# Compile Definitions
########################################
add_compile_definitions(${rcsw_LIBRARY}
  PRIVATE
  RCSW_STDIO_PUTCHAR=myputchar
)
# add_compile_definitions(${rcsw_LIBRARY}
#   PRIVATE
#   RCSW_STDIO_GETCHAR=mygetchar
# )

########################################
# Include directories
########################################
target_include_directories(
  ${rcsw_LIBRARY}
  SYSTEM PUBLIC
  $<BUILD_INTERFACE:${rcsw_DIR}/ext>
  )
target_include_directories(
  ${rcsw_LIBRARY}
  PUBLIC
  $<BUILD_INTERFACE:${rcsw_DIR}/include>
  )

########################################
# Link Libraries
########################################
target_link_libraries(${rcsw_LIBRARY} pthread dl m)

################################################################################
# Installation and Deployment
################################################################################
# Installation
libra_configure_exports_as(${rcsw_LIBRARY} ${CMAKE_INSTALL_PREFIX})
libra_register_target_for_install(${rcsw_LIBRARY} ${CMAKE_INSTALL_PREFIX})
libra_register_headers_for_install(include/${rcsw_LIBRARY} ${CMAKE_INSTALL_PREFIX})


# Deployment
if(NOT CPACK_PACKAGE_NAME)
  set(CPACK_PACKAGE_NAME ${rcsw_LIBRARY})
endif()

libra_register_copyright_for_install(${rcsw_LIBRARY} ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/changelog")
  libra_register_changelog_for_install(${rcsw_LIBRARY} ${CMAKE_CURRENT_SOURCE_DIR}/changelog)
endif()


libra_configure_cpack(
  "DEB"

  "Collection of reusable C software modules for embedded programming, styled
after the C++ STL.

It has many data structures, a publisher-subscriber system, concurrent
programming utilities, and simple I/O routines for bare-metal applications."

  "John Harwell"
  "https://jharwell.github.io/rcsw"
  "John Harwell <john.r.harwell@gmail.com>")

################################################################################
# Status
################################################################################
libra_config_summary()

message("")
message("--------------------------------------------------------------------------------")
message("                           RCSW Configuration Summary")
message("--------------------------------------------------------------------------------")
message("")

message(STATUS "Version                               : rcsw_VERSION=${rcsw_VERSION}")
message("")
message("--------------------------------------------------------------------------------")
