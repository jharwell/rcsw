################################################################################
# Configuration Options                                                        #
################################################################################
set(rcsw_CHECK_LANGUAGE "C")

################################################################################
# Components                                                                   #
################################################################################
component_register_as_src(
  rcsw_adapter_SRC
  rcsw
  "${rcsw_SRC}"
  adapter
  "src/adapter")

component_register_as_src(
  rcsw_algorithm_SRC
  rcsw
  "${rcsw_SRC}"
  algorithm
  "src/algorithm")
component_register_as_src(
  rcsw_common_SRC
  rcsw
  "${rcsw_SRC}"
  common
  "src/common")
component_register_as_src(
  rcsw_ds_SRC
  rcsw
  "${rcsw_SRC}"
  ds
  "src/ds")

component_register_as_src(
  rcsw_mp_common_SRC
  rcsw
  "${rcsw_SRC}"
  mp_common
  "src/multiprocess/procm")
  # If we don't do this, then we have to use the mpicc/mpicxx C/C++
  # compiler wrappers even if we are not using MPI. (Slightly) better to
  # do it this way I think.
if (LIBRA_MPI)
  component_register_as_src(
    rcsw_mp_mpi_SRC
    rcsw
    "${rcsw_SRC}"
    mp_mpi
    "src/multiprocess/mpi")
endif()
component_register_as_src(
  rcsw_multithread_SRC
  rcsw
  "${rcsw_SRC}"
  multithread
  "src/multithread")
component_register_as_src(
  rcsw_pulse_SRC
  rcsw
"${rcsw_SRC}"
  pulse
  "src/pulse")
component_register_as_src(
  rcsw_stdio_SRC
  rcsw
  "${rcsw_SRC}"
  stdio
  "src/stdio")
component_register_as_src(
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

requested_components_check(rcsw)

################################################################################
# Libraries                                                                    #
################################################################################
# Create the source for the SINGLE library to build by combining the
# source of the selected components
foreach(component ${rcsw_FIND_COMPONENTS})
  if(${rcsw_${component}_FOUND})
    list(APPEND rcsw_components_SRC ${rcsw_} ${rcsw_${component}_SRC})
  endif()
endforeach()

add_library(
  rcsw
  STATIC
  ${rcsw_components_SRC}
  )

target_link_libraries(rcsw pthread dl)

target_include_directories(
  rcsw
  SYSTEM PUBLIC
  $<BUILD_INTERFACE:${rcsw_DIR}/ext>
  )
target_include_directories(
  rcsw
  PUBLIC
  $<BUILD_INTERFACE:${rcsw_DIR}/include>
  )

################################################################################
# Installation                                                                 #
################################################################################
configure_exports_as(rcsw ${CMAKE_INSTALL_PREFIX})
register_target_for_install(rcsw ${CMAKE_INSTALL_PREFIX})
register_headers_for_install(include/rcsw ${CMAKE_INSTALL_PREFIX})

################################################################################
# Status                                                                       #
################################################################################
libra_config_summary()
