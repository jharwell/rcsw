################################################################################
# Configuration Options
################################################################################
# We might be linking with a shared library
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(rcsw_CHECK_LANGUAGE "C")

set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 2)
set(PROJECT_VERSION_PATCH 17)
set(rcsw_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

libra_configure_version(
  ${CMAKE_CURRENT_SOURCE_DIR}/src/version/version.c.in
  ${CMAKE_CURRENT_BINARY_DIR}/src/version/version.c
  rcsw_components_SRC
)

# Are we allowed to allocate memory?
if (NOT RCSW_CONFIG_NOALLOC)
  set(RCSW_CONFIG_NOALLOC NO)
endif()

# Should all memory be zeroed before use (regardless of where it came
# from) ?
if (NOT RCSW_CONFIG_ZALLOC)
  set(RCSW_CONFIG_ZALLOC NO)
endif()

if (NOT RCSW_CONFIG_STDIO_PRINTF_BUFSIZE)
  set(RCSW_CONFIG_STDIO_PRINTF_BUFSIZE 32)
endif()


if (NOT RCSW_CONFIG_STDIO_PRINTF_WITH_DEC)
  set(RCSW_CONFIG_STDIO_PRINTF_WITH_DEC YES)
endif()

if (NOT RCSW_CONFIG_STDIO_PRINTF_WITH_EXP)
  set(RCSW_CONFIG_STDIO_PRINTF_WITH_EXP YES)
endif()

if (NOT RCSW_CONFIG_STDIO_PRINTF_WITH_WRITEBACK)
  set(RCSW_CONFIG_STDIO_PRINTF_WITH_WRITEBACK YES)
endif()


if (NOT RCSW_CONFIG_STDIO_PRINTF_DEFAULT_FLOAT_PREC)
  set(RCSW_CONFIG_STDIO_PRINTF_DEFAULT_FLOAT_PREC 6)
endif()


if (NOT RCSW_CONFIG_STDIO_PRINTF_EXP_DIGIT_THRESH)
  set(RCSW_CONFIG_STDIO_PRINTF_EXP_DIGIT_THRESH 9)
endif()


if (NOT RCSW_CONFIG_STDIO_PRINTF_WITH_LL)
  set(RCSW_CONFIG_STDIO_PRINTF_WITH_LL YES)
endif()


if (NOT RCSW_CONFIG_STDIO_MATH_LOG10_TERMS)
  set(RCSW_CONFIG_STDIO_MATH_LOG10_TERMS 4)
endif()


if (NOT RCSW_CONFIG_STDIO_PRINTF_CHECK_NULL)
  set(RCSW_CONFIG_STDIO_PRINTF_CHECK_NULL YES)
endif()

if(NOT RCSW_CONFIG_NO_GRIND)
  set(RCSW_CONFIG_NO_GRIND NO)
endif()

set (RCSW_ONOFF_CONFIG
  RCSW_CONFIG_STDIO_PRINTF_WITH_DEC
  RCSW_CONFIG_STDIO_PRINTF_WITH_EXP
  RCSW_CONFIG_STDIO_PRINTF_WITH_WRITEBACK
  RCSW_CONFIG_STDIO_PRINTF_WITH_LL
  RCSW_CONFIG_STDIO_PRINTF_CHECK_NULL
  RCSW_CONFIG_NO_GRIND
)
set (RCSW_VALUE_CONFIG
  RCSW_CONFIG_STDIO_PRINTF_BUFSIZE
  RCSW_CONFIG_STDIO_PRINTF_DEFAULT_FLOAT_PREC
  RCSW_CONFIG_STDIO_PRINTF_EXP_DIGIT_THRESH
  RCSW_CONFIG_STDIO_MATH_LOG10_TERMS
)

if (NOT RCSW_CONFIG_STDIO_PUTCHAR)
  set(RCSW_CONFIG_STDIO_PUTCHAR putchar)
endif()
if (NOT RCSW_CONFIG_STDIO_GETCHAR)
  set(RCSW_CONFIG_STDIO_GETCHAR getchar)
endif()

if(NOT RCSW_CONFIG_ER_PLUGIN)
  set(RCSW_CONFIG_ER_PLUGIN  LOG4CL)
endif()

if(NOT LIBRA_FPC)
  set(LIBRA_FPC  RETURN)
endif()

if(NOT RCSW_CONFIG_PTR_ALIGN)
  # We know x86 can handle 1 byte-aligned addresses; add other
  # architectures here as needed.
  if("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "x86_64")
    set(RCSW_CONFIG_PTR_ALIGN 4)
    # We know ARM can't handle 1 byte-aligned accesses from say a
    # 4-byte aligned pointer.
  elseif("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "arm")
    set(RCSW_CONFIG_PTR_ALIGN 1)
  else()
    # The fallback is the (possibly) less-efficient 1 byte alignment.
    set(RCSW_CONFIG_PTR_ALIGN 1)
    message(WARNING "Novel build target architecture '${CMAKE_SYSTEM_PROCESSOR}'--default to byte aligned data storage")
  endif()
endif()

if (NOT RCSW_CONFIG_SUMMARY)
  set(RCSW_CONFIG_SUMMARY YES)
endif()

if(NOT RCSW_CONFIG_LIBTYPE)
  set(RCSW_CONFIG_LIBTYPE STATIC)
endif()

################################################################################
# Components
################################################################################
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


# If we don't do this, then we have to use the mpicc/mpicxx C/C++
# compiler wrappers even if we are not using MPI. (Slightly) better to
# do it this way I think.
if ("${LIBRA_MP}")
  libra_component_register_as_src(
    rcsw_multiprocess_SRC
    rcsw
    "${rcsw_SRC}"
    multiprocess
    "src/multiprocess")
endif()

if ("${LIBRA_MT}")
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
endif()

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
    multithread
    multiprocess
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
  ${RCSW_CONFIG_LIBTYPE}
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
target_compile_definitions(${rcsw_LIBRARY}
  PUBLIC
  RCSW_STDIO_PUTCHAR=${RCSW_CONFIG_STDIO_PUTCHAR}
)

target_compile_definitions(${rcsw_LIBRARY}
  PUBLIC
  LIBRA_ERL=LIBRA_ERL_${LIBRA_ERL}
)

target_compile_definitions(${rcsw_LIBRARY}
  PUBLIC
  RCSW_ER_PLUGIN=RCSW_ER_PLUGIN_${RCSW_CONFIG_ER_PLUGIN}
)
target_compile_definitions(${rcsw_LIBRARY}
  PUBLIC
  RCSW_ER_PLUGIN_PATH=${RCSW_CONFIG_ER_PLUGIN_PATH}
)
target_compile_definitions(${rcsw_LIBRARY}
  PUBLIC
  LIBRA_FPC=LIBRA_FPC_${LIBRA_FPC}
)

target_compile_definitions(${rcsw_LIBRARY}
  PUBLIC
  RCSW_PTR_ALIGN=${RCSW_CONFIG_PTR_ALIGN}
)

if("${RCSW_CONFIG_NOALLOC}")
  target_compile_definitions(${rcsw_LIBRARY}
    PUBLIC
    RCSW_CONFIG_NOALLOC
  )
endif()

if("${RCSW_CONFIG_ZALLOC}")
  target_compile_definitions(${rcsw_LIBRARY}
    PUBLIC
    RCSW_CONFIG_ZALLOC
  )
endif()


foreach(config ${RCSW_ONOFF_CONFIG})
  if(${config})
    target_compile_definitions(${rcsw_LIBRARY}
      PUBLIC
      ${config}
    )
  endif()
endforeach()

foreach(config ${RCSW_VALUE_CONFIG})
  if(${config})
    target_compile_definitions(${rcsw_LIBRARY}
      PUBLIC
      ${config}=${${config}}
    )
  endif()
endforeach()

get_target_property(target_type ${rcsw_LIBRARY} TYPE)
if (target_type STREQUAL SHARED_LIBRARY)
  target_compile_definitions(${rcsw_LIBRARY}
    PRIVATE
    RCSW_EXPORTS=1
  )
  set_target_properties(${rcsw_LIBRARY} PROPERTIES
    C_VISIBILITY_PRESET hidden
  )
endif()

########################################
# Include directories
########################################
target_include_directories(
  ${rcsw_LIBRARY}
  PUBLIC
  $<BUILD_INTERFACE:${rcsw_DIR}/include>
  ${LIBRA_DEPS_PREFIX}/include
)

########################################
# Link Libraries
########################################
target_link_libraries(${rcsw_LIBRARY} pthread dl m)

if("${RCSW_CONFIG_ER_PLUGIN}" STREQUAL "ZLOG")
  target_link_libraries(${rcsw_LIBRARY} zlog)
  target_link_directories(${rcsw_LIBRARY} PUBLIC ${LIBRA_DEPS_PREFIX}/lib)
endif()

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

It has many data structures, a publisher-subscriber system,
plugin-based event reporting framework, and a simple stdlib replacement
for bare-metal applications."

  "John Harwell"
  "https://jharwell.github.io/rcsw"
  "John Harwell <john.r.harwell@gmail.com>")

################################################################################
# Status
################################################################################
if(${RCSW_CONFIG_SUMMARY})

  message("")
  message("${BoldBlue}--------------------------------------------------------------------------------")
  message("${BoldBlue}                           RCSW Configuration Summary")
  message("${BoldBlue}--------------------------------------------------------------------------------")
  message("")


  set(fields
    rcsw_VERSION
    RCSW_CONFIG_LIBTYPE
    RCSW_CONFIG_ER_PLUGIN
    RCSW_CONFIG_ER_PLUGIN_PATH
    RCSW_CONFIG_PTR_ALIGN
    RCSW_CONFIG_NOALLOC
    RCSW_CONFIG_ZALLOC
    RCSW_CONFIG_STDIO_GETCHAR
    RCSW_CONFIG_STDIO_PUTCHAR
    RCSW_CONFIG_STDIO_MATH_LOG10_TERMS
    RCSW_CONFIG_STDIO_PRINTF_BUFSIZE
    RCSW_CONFIG_STDIO_PRINTF_WITH_DEC
    RCSW_CONFIG_STDIO_PRINTF_WITH_EXP
    RCSW_CONFIG_STDIO_PRINTF_WITH_WRITEBACK
    RCSW_CONFIG_STDIO_PRINTF_DEFAULT_FLOAT_PREC
    RCSW_CONFIG_STDIO_PRINTF_EXP_DIGIT_THRESH
    RCSW_CONFIG_STDIO_PRINTF_WITH_LL
    RCSW_CONFIG_STDIO_PRINTF_CHECK_NULL
    RCSW_CONFIG_NO_GRIND
  )
  libra_config_summary_prepare_fields("${fields}")

  message(STATUS "Version                                       : ${ColorBold}${EMIT_rcsw_VERSION}${ColorReset} [rcsw_VERSION]")
  message(STATUS "Library type                                  : ${ColorBold}${EMIT_RCSW_CONFIG_LIBTYPE}${ColorReset} [RCSW_CONFIG_LIBTYPE={STATIC,SHARED}]")
  message(STATUS "Event reporting plugin                        : ${ColorBold}${EMIT_RCSW_CONFIG_ER_PLUGIN}${ColorReset} [RCSW_CONFIG_ER_PLUGIN={CUSTOM,ZLOG,LOG4CL,SIMPLE}]")
  message(STATUS "Event reporting custom plugin path            : ${ColorBold}${EMIT_RCSW_CONFIG_ER_PLUGIN_PATH}${ColorReset} [RCSW_CONFIG_ER_PLUGIN_PATH]")
  message(STATUS "Disable dynamic memory allocation             : ${ColorBold}${EMIT_RCSW_CONFIG_NOALLOC}${ColorReset} [RCSW_CONFIG_NOALLOC]")
  message(STATUS "Always zero alloc'd memory before use         : ${ColorBold}${EMIT_RCSW_CONFIG_ZALLOC}${ColorReset} [RCSW_CONFIG_ZALLOC]")
  message(STATUS "Data pointer alignment                        : ${ColorBold}${EMIT_RCSW_CONFIG_PTR_ALIGN}${ColorReset} [RCSW_CONFIG_PTR_ALIGN={1,2,4}]")
  message(STATUS "stdio getchar() function                      : ${ColorBold}${EMIT_RCSW_CONFIG_STDIO_GETCHAR}${ColorReset} [RCSW_CONFIG_STDIO_GETCHAR]")
  message(STATUS "stdio putchar() function                      : ${ColorBold}${EMIT_RCSW_CONFIG_STDIO_PUTCHAR}${ColorReset} [RCSW_CONFIG_STDIO_PUTCHAR]")
  message(STATUS "stdio math taylor expansion terms             : ${ColorBold}${EMIT_RCSW_CONFIG_STDIO_MATH_LOG10_TERMS}${ColorReset} [RCSW_CONFIG_STDIO_MATH_LOG10_TERMS]")
  message(STATUS "stdio printf() buffer size                    : ${ColorBold}${EMIT_RCSW_CONFIG_STDIO_PRINTF_BUFSIZE}${ColorReset} [RCSW_CONFIG_STDIO_PRINTF_BUFSIZE]")
  message(STATUS "stdio printf() support for decimals           : ${ColorBold}${EMIT_RCSW_CONFIG_STDIO_PRINTF_WITH_DEC}${ColorReset} [RCSW_CONFIG_STDIO_PRINTF_WITH_DEC]")
  message(STATUS "stdio printf() support for exponentials       : ${ColorBold}${EMIT_RCSW_CONFIG_STDIO_PRINTF_WITH_EXP}${ColorReset} [RCSW_CONFIG_STDIO_PRINTF_WITH_EXP]")
  message(STATUS "stdio printf() support for writeback          : ${ColorBold}${EMIT_RCSW_CONFIG_STDIO_PRINTF_WITH_WRITEBACK}${ColorReset} [RCSW_CONFIG_STDIO_PRINTF_WITH_WRITEBACK]")
  message(STATUS "stdio printf() default float precision        : ${ColorBold}${EMIT_RCSW_CONFIG_STDIO_PRINTF_DEFAULT_FLOAT_PREC}${ColorReset} [RCSW_CONFIG_STDIO_PRINTF_DEFAULT_FLOAT_PREC]")
  message(STATUS "stdio printf() decimal -> exp digit threshold : ${ColorBold}${EMIT_RCSW_CONFIG_STDIO_PRINTF_EXP_DIGIT_THRESH}${ColorReset} [RCSW_CONFIG_STDIO_PRINTF_EXP_DIGIT_THRESH]")
  message(STATUS "stdio printf() support for long long          : ${ColorBold}${EMIT_RCSW_CONFIG_STDIO_PRINTF_WITH_LL}${ColorReset} [RCSW_CONFIG_STDIO_PRINTF_WITH_LL]")
  message(STATUS "stdio printf() fmt string safety check        : ${ColorBold}${EMIT_RCSW_CONFIG_STDIO_PRINTF_CHECK_NULL}${ColorReset} [RCSW_CONFIG_STDIO_PRINTF_CHECK_NULL]")
  message(STATUS "Compile out RCSW_GRIND_XX() macros            : ${ColorBold}${EMIT_RCSW_CONFIG_NO_GRIND}${ColorReset} [RCSW_CONFIG_NO_GRIND]")
  message("")
  message("${BoldBlue}--------------------------------------------------------------------------------${ColorReset}")

endif()
