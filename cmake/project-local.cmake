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

# 'ntoa' conversion buffer size, this must be big enough to hold one
# converted numeric number including padded zeros (dynamically created
# on stack)
if (NOT RCSW_STDIO_PRINTF_BUFFER_SIZE)
  set(RCSW_STDIO_PRINTF_BUFFER_SIZE 32)
endif()

# Support for the decimal notation floating point conversion specifiers (%f,
# %F)
if (NOT RCSW_STDIO_PRINTF_SUPPORT_DEC)
  set(RCSW_STDIO_PRINTF_SUPPORT_DEC YES)
endif()

# Support for the exponential notation floating point conversion specifiers
# (%e, %g, %E, %G)
if (NOT RCSW_STDIO_PRINTF_SUPPORT_EXP)
  set(RCSW_STDIO_PRINTF_SUPPORT_EXP YES)
endif()

 # Support for the length write-back specifier (%n)
if (NOT RCSW_STDIO_PRINTF_SUPPORT_WRITEBACK)
  set(RCSW_STDIO_PRINTF_SUPPORT_WRITEBACK YES)
endif()

# Default precision for the floating point conversion specifiers (the C
# standard sets this at 6)
if (NOT RCSW_STDIO_PRINTF_DEFAULT_FLOAT_PREC)
  set(RCSW_STDIO_PRINTF_DEFAULT_FLOAT_PREC 6)
endif()

# According to the C languages standard, printf() and related
# functions must be able to print any integral number in
# floating-point notation, regardless of length, when using the %f
# specifier - possibly hundreds of characters, potentially overflowing
# your buffers. In this implementation, all values beyond this
# threshold are switched to exponential notation.
if (NOT RCSW_STDIO_PRINTF_EXP_DIGIT_THRESH)
  set(RCSW_STDIO_PRINTF_EXP_DIGIT_THRESH 9)
endif()

# Support for the long long integral types (with the ll, z and t
# length modifiers for specifiers %d,%i,%o,%x,%X,%u, and with the %p
# specifier). Note: 'L' (long double) is not supported.
if (NOT RCSW_STDIO_PRINTF_SUPPORT_LONG_LONG)
  set(RCSW_STDIO_PRINTF_SUPPORT_LONG_LONG YES)
endif()

# The number of terms in a Taylor series expansion of log_10(x) to
# use for approximation - including the power-zero term (i.e. the
# value at the point of expansion).
if (NOT RCSW_STDIO_MATH_LOG10_TAYLOR_TERMS)
  set(RCSW_STDIO_MATH_LOG10_TAYLOR_TERMS 4)
endif()

# Be extra-safe, and don't assume format specifiers are completed correctly
# before the format string end.
if (NOT RCSW_STDIO_PRINTF_CHECK_NULL)
  set(RCSW_STDIO_PRINTF_CHECK_NULL YES)
endif()


set (RCSW_STDIO_ONOFF_CONFIG
  RCSW_STDIO_PRINTF_SUPPORT_DEC
  RCSW_STDIO_PRINTF_SUPPORT_EXP
  RCSW_STDIO_PRINTF_SUPPORT_WRITEBACK
  RCSW_STDIO_PRINTF_SUPPORT_LONG_LONG
  RCSW_STDIO_PRINTF_CHECK_NULL
)
set (RCSW_STDIO_VALUE_CONFIG
  RCSW_STDIO_PRINTF_BUFFER_SIZE
  RCSW_STDIO_PRINTF_DEFAULT_FLOAT_PREC
  RCSW_STDIO_PRINTF_EXP_DIGIT_THRESH
  RCSW_STDIO_MATH_LOG10_TAYLOR_TERMS
)

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
    er
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

target_compile_definitions(${rcsw_LIBRARY}
  INTERFACE
  LIBRA_ER=LIBRA_ER_${LIBRA_ER}
)
add_compile_definitions(${rcsw_LIBRARY}
  INTERFACE
  RCSW_ER_PLUGIN=RCSW_ER_PLUGIN_${RCSW_ER_PLUGIN}
)

foreach(config ${RCSW_STDIO_ONOFF_CONFIG})
  if(${config})
      add_compile_definitions(${rcsw_LIBRARY}
        PRIVATE
        ${config}
      )
    endif()
endforeach()

foreach(config ${RCSW_STDIO_VALUE_CONFIG})
  if(${config})
      add_compile_definitions(${rcsw_LIBRARY}
        PRIVATE
        ${config}=${${config}}
      )
    endif()
endforeach()


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

message(STATUS "Version                                       : rcsw_VERSION=${rcsw_VERSION}")
message(STATUS "Event reporting plugin                        : RCSW_ER_PLUGIN=${RCSW_ER_PLUGIN}")
message(STATUS "stdio getchar() function                      : RCSW_STDIO_GETCHAR=${RCSW_STDIO_GETCHAR}")
message(STATUS "stdio putchar() function                      : RCSW_STDIO_PUTCHAR=${RCSW_STDIO_PUTCHAR}")
message(STATUS "stdio math taylor expansion terms             : RCSW_STDIO_MATH_LOG10_TAYLOR_TERMS=${RCSW_STDIO_MATH_LOG10_TAYLOR_TERMS}")
message(STATUS "stdio printf() buffer size                    : RCSW_STDIO_PRINTF_BUFFER_SIZE=${RCSW_STDIO_PRINTF_BUFFER_SIZE}")
message(STATUS "stdio printf() support for decimals           : RCSW_STDIO_PRINTF_SUPPORT_DEC=${RCSW_STDIO_PRINTF_SUPPORT_DEC}")
message(STATUS "stdio printf() support for exponentials       : RCSW_STDIO_PRINTF_SUPPORT_EXP=${RCSW_STDIO_PRINTF_SUPPORT_EXP}")
message(STATUS "stdio printf() support for writeback          : RCSW_STDIO_PRINTF_SUPPORT_WRITEBACK=${RCSW_STDIO_PRINTF_SUPPORT_WRITEBACK}")
message(STATUS "stdio printf() default float precision        : RCSW_STDIO_PRINTF_DEFAULT_FLOAT_PREC=${RCSW_STDIO_PRINTF_DEFAULT_FLOAT_PREC}")
message(STATUS "stdio printf() decimal -> exp digit threshold : RCSW_STDIO_PRINTF_EXP_DIGIT_THRESH=${RCSW_STDIO_PRINTF_EXP_DIGIT_THRESH}")
message(STATUS "stdio printf() support for long long          : RCSW_STDIO_PRINTF_SUPPORT_LONG_LONG=${RCSW_STDIO_PRINTF_SUPPORT_LONG_LONG}")
message(STATUS "stdio printf() safety check in fmt strings    : RCSW_STDIO_PRINTF_CHECK_NULL=${RCSW_STDIO_PRINTF_CHECK_NULL}")

# add_compile_definitions(${rcsw_LIBRARY}
#   PRIVATE
#   RCSW_STDIO_GETCHAR=mygetchar
# )
message("")
message("--------------------------------------------------------------------------------")
