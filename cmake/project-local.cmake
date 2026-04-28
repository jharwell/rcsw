# ##############################################################################
# Packages
# ##############################################################################
cpmaddpackage(
  NAME
  Catch2
  GITHUB_REPOSITORY
  catchorg/Catch2
  VERSION
  3.7.0)

list(APPEND CMAKE_PREFIX_PATH ${catch2_BINARY_DIR})

set(LIBRA_TEST_HARNESS_LIBS Catch2::Catch2WithMain)

# ##############################################################################
# User-facing options and cache variables
# ##############################################################################
set(RCSW_BUILD_FOR
    "POSIX"
    CACHE STRING "Target platform. One of: POSIX, BAREMETAL")
set_property(CACHE RCSW_BUILD_FOR PROPERTY STRINGS POSIX BAREMETAL)

set(RCSW_CONFIG_LIBTYPE
    STATIC
    CACHE STRING "Library type. One of: STATIC, SHARED")
set_property(CACHE RCSW_CONFIG_LIBTYPE PROPERTY STRINGS STATIC SHARED)

set(RCSW_CONFIG_ER_PLUGIN
    LOG4CL
    CACHE STRING "Event reporting plugin. One of: LOG4CL, ZLOG, SIMPLE")
set_property(CACHE RCSW_CONFIG_ER_PLUGIN PROPERTY STRINGS LOG4CL ZLOG SIMPLE)

set(RCSW_CONFIG_PTR_ALIGN
    ""
    CACHE STRING
          "Data pointer alignment in bytes (1, 2, 4). Auto-detected if empty.")

option(RCSW_CONFIG_NO_STDIO "Exclude the STDIO module from the build" OFF)
option(RCSW_CONFIG_NOALLOC "Disable dynamic memory allocation" OFF)
option(RCSW_CONFIG_ZALLOC "Zero all allocated memory before use" OFF)
option(RCSW_CONFIG_TOOL_NO_GRIND "Compile out RCSW_GRIND_XX() macros" OFF)
option(RCSW_CONFIG_MP "Build multiprocess support (requires MPI)" OFF)

# STDIO tunables — only relevant when RCSW_CONFIG_NO_STDIO=OFF
set(RCSW_CONFIG_STDIO_PUTCHAR
    putchar
    CACHE STRING "stdio putchar() replacement function")
set(RCSW_CONFIG_STDIO_GETCHAR
    getchar
    CACHE STRING "stdio getchar() replacement function")
set(RCSW_CONFIG_STDIO_PRINTF_BUFSIZE
    32
    CACHE STRING "printf() internal buffer size")
set(RCSW_CONFIG_STDIO_PRINTF_DEFAULT_FLOAT_PREC
    6
    CACHE STRING "printf() default float precision")
set(RCSW_CONFIG_STDIO_PRINTF_EXP_DIGIT_THRESH
    9
    CACHE STRING "printf() decimal-to-exponential digit threshold")
set(RCSW_CONFIG_STDIO_MATH_LOG10_TERMS
    4
    CACHE STRING "Taylor expansion terms for log10()")
option(RCSW_CONFIG_STDIO_PRINTF_WITH_DEC "printf() decimal support" ON)
option(RCSW_CONFIG_STDIO_PRINTF_WITH_EXP "printf() exponential support" ON)
option(RCSW_CONFIG_STDIO_PRINTF_WITH_WRITEBACK "printf() writeback support" ON)
option(RCSW_CONFIG_STDIO_PRINTF_WITH_LL "printf() long long support" ON)
option(RCSW_CONFIG_STDIO_PRINTF_CHECK_NULL "printf() format string safety" ON)

# ##############################################################################
# Platform-specific overrides
# ##############################################################################
if("${RCSW_BUILD_FOR}" MATCHES "POSIX")
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
elseif("${RCSW_BUILD_FOR}" MATCHES "BAREMETAL")
  # Shared libraries don't make sense on bare-metal/freestanding targets
  set(RCSW_CONFIG_LIBTYPE
      STATIC
      CACHE STRING "" FORCE)
  set(CMAKE_POSITION_INDEPENDENT_CODE OFF)

  if(${LIBRA_STDLIB} MATCHES "NONE")
    set(RCSW_CONFIG_NOALLOC YES)
    set(RCSW_CONFIG_ER_PLUGIN
        SIMPLE
        CACHE STRING "" FORCE)
    if(${RCSW_CONFIG_NO_STDIO})
      message(
        FATAL_ERROR
          "RCSW requires RCSW_CONFIG_NO_STDIO=NO when building for baremetal without the standard library."
      )
    endif()
  endif()
else()
  message(FATAL_ERROR "RCSW_BUILD_FOR must be one of: POSIX, BAREMETAL")
endif()

# Pointer alignment auto-detection
if(NOT RCSW_CONFIG_PTR_ALIGN)
  if("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "x86_64")
    set(RCSW_CONFIG_PTR_ALIGN 4)
  elseif("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "arm")
    set(RCSW_CONFIG_PTR_ALIGN 1)
  else()
    set(RCSW_CONFIG_PTR_ALIGN 1)
    message(
      WARNING
        "Novel build target architecture '${CMAKE_SYSTEM_PROCESSOR}' -- defaulting to byte-aligned data storage"
    )
  endif()
endif()

# ##############################################################################
# Sources
# ##############################################################################
libra_configure_source_file(
  ${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/src/version/version.c.in
  ${CMAKE_CURRENT_BINARY_DIR}/src/version/version.c rcsw_components_SRC)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/collect.cmake)
if("${RCSW_BUILD_FOR}" MATCHES "BAREMETAL")
  rcsw_baremetal_collect_sources(rcsw_components_SRC)
elseif("${RCSW_BUILD_FOR}" MATCHES "POSIX")
  rcsw_posix_collect_sources(rcsw_components_SRC)
endif()

# ##############################################################################
# Library target
# ##############################################################################
libra_add_library(${PROJECT_NAME} ${RCSW_CONFIG_LIBTYPE} ${rcsw_components_SRC})

# Available whether installed or used as a subdirectory via CPM
add_library(rcsw::rcsw ALIAS rcsw)

# Embeds version in the filename; creates an unversioned symlink for shared libs
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION}
                                                 SOVERSION ${PROJECT_VERSION})

# ##############################################################################
# Compile definitions
# ##############################################################################
target_compile_definitions(
  ${PROJECT_NAME}
  PRIVATE RCSW_CONFIG_ER_PLUGIN=RCSW_ER_PLUGIN_${RCSW_CONFIG_ER_PLUGIN}
  PUBLIC RCSW_CONFIG_AL_TARGET=RCSW_AL_TARGET_${RCSW_BUILD_FOR})

# Boolean options: define the symbol only when ON
foreach(
  config IN
  ITEMS RCSW_CONFIG_STDIO_PRINTF_WITH_DEC
        RCSW_CONFIG_STDIO_PRINTF_WITH_EXP
        RCSW_CONFIG_STDIO_PRINTF_WITH_WRITEBACK
        RCSW_CONFIG_STDIO_PRINTF_WITH_LL
        RCSW_CONFIG_STDIO_PRINTF_CHECK_NULL
        RCSW_CONFIG_TOOL_NO_GRIND
        RCSW_CONFIG_NOALLOC
        RCSW_CONFIG_ZALLOC)
  if(${config})
    target_compile_definitions(${PROJECT_NAME} PRIVATE ${config})
  endif()
endforeach()

# Value options: define as NAME=VALUE
foreach(config IN ITEMS RCSW_CONFIG_STDIO_PUTCHAR RCSW_CONFIG_STDIO_GETCHAR
                        RCSW_CONFIG_PTR_ALIGN)
  target_compile_definitions(${PROJECT_NAME} PUBLIC ${config}=${${config}})
endforeach()

foreach(
  config IN
  ITEMS RCSW_CONFIG_STDIO_PRINTF_BUFSIZE
        RCSW_CONFIG_STDIO_PRINTF_DEFAULT_FLOAT_PREC
        RCSW_CONFIG_STDIO_PRINTF_EXP_DIGIT_THRESH
        RCSW_CONFIG_STDIO_MATH_LOG10_TERMS)
  target_compile_definitions(${PROJECT_NAME} PRIVATE ${config}=${${config}})
endforeach()

get_target_property(target_type ${PROJECT_NAME} TYPE)
if(target_type STREQUAL SHARED_LIBRARY)
  set_target_properties(${PROJECT_NAME} PROPERTIES C_VISIBILITY_PRESET hidden)
endif()

# ##############################################################################
# Include directories
# ##############################################################################
target_include_directories(
  ${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${rcsw_DIR}/include>
                         $<INSTALL_INTERFACE:include>)

# ##############################################################################
# Link libraries
# ##############################################################################
target_link_libraries(${PROJECT_NAME} PUBLIC pthread dl m)

if("${RCSW_CONFIG_ER_PLUGIN}" STREQUAL "ZLOG")
  target_link_libraries(${PROJECT_NAME} PUBLIC zlog)
endif()

# ##############################################################################
# Installation and deployment
# ##############################################################################
# Only applies to POSIX/Linux; embedding targets don't install.
if("${RCSW_BUILD_FOR}" MATCHES "POSIX")
  libra_configure_exports(${PROJECT_NAME})
  libra_install_target(${PROJECT_NAME} INCLUDE_DIR include/${PROJECT_NAME})
  libra_install_copyright(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE)

  if(NOT CPACK_PACKAGE_NAME)
    set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
  endif()

  set(RCSW_PKG_SUMMARY
      "Collection of Reusable C SoftWare (RCSW) modules for embedded programming"
  )

  set(RCSW_PKG_DESCRIPTION
      "Collection of reusable C software modules for embedded programming,\
styled after the C++ STL. Features:\n\
\n\
* Many data structures (lists, queues, trees, hash tables)\n\
* Publisher-subscriber system\n\
* Plugin-based event reporting framework\n\
* Simple stdlib replacement for bare-metal applications\n\
\n\
This is a ${RCSW_CONFIG_LIBTYPE} library, built for ${RCSW_BUILD_FOR}:\n\
  * RCSW_CONFIG_ER_PLUGIN=${RCSW_CONFIG_ER_PLUGIN}\n\
  * RCSW_CONFIG_PTR_ALIGN=${RCSW_CONFIG_PTR_ALIGN}\n\
  * RCSW_CONFIG_NOALLOC=${RCSW_CONFIG_NOALLOC}\n\
  * RCSW_CONFIG_ZALLOC=${RCSW_CONFIG_ZALLOC}\n\
  * RCSW_CONFIG_TOOL_NO_GRIND=${RCSW_CONFIG_TOOL_NO_GRIND}\n\
  * RCSW_CONFIG_NO_STDIO=${RCSW_CONFIG_NO_STDIO}\n\
  * RCSW_CONFIG_STDIO_GETCHAR=${RCSW_CONFIG_STDIO_GETCHAR}\n\
  * RCSW_CONFIG_STDIO_PUTCHAR=${RCSW_CONFIG_STDIO_PUTCHAR}\n\
  * RCSW_CONFIG_STDIO_MATH_LOG10_TERMS=${RCSW_CONFIG_STDIO_MATH_LOG10_TERMS}\n\
  * RCSW_CONFIG_STDIO_PRINTF_BUFSIZE=${RCSW_CONFIG_STDIO_PRINTF_BUFSIZE}\n\
  * RCSW_CONFIG_STDIO_PRINTF_WITH_DEC=${RCSW_CONFIG_STDIO_PRINTF_WITH_DEC}\n\
  * RCSW_CONFIG_STDIO_PRINTF_WITH_EXP=${RCSW_CONFIG_STDIO_PRINTF_WITH_EXP}\n\
  * RCSW_CONFIG_STDIO_PRINTF_WITH_WRITEBACK=${RCSW_CONFIG_STDIO_PRINTF_WITH_WRITEBACK}\n\
  * RCSW_CONFIG_STDIO_PRINTF_DEFAULT_FLOAT_PREC=${RCSW_CONFIG_STDIO_PRINTF_DEFAULT_FLOAT_PREC}\n\
  * RCSW_CONFIG_STDIO_PRINTF_EXP_DIGIT_THRESH=${RCSW_CONFIG_STDIO_PRINTF_EXP_DIGIT_THRESH}\n\
  * RCSW_CONFIG_STDIO_PRINTF_WITH_LL=${RCSW_CONFIG_STDIO_PRINTF_WITH_LL}\n\
  * RCSW_CONFIG_STDIO_PRINTF_CHECK_NULL=${RCSW_CONFIG_STDIO_PRINTF_CHECK_NULL}")

  libra_configure_cpack(
    "DEB;RPM"
    ${RCSW_PKG_SUMMARY}
    ${RCSW_PKG_DESCRIPTION}
    "John Harwell"
    "https://jharwell.github.io/rcsw"
    "John Harwell <john.r.harwell@gmail.com>")
endif()

# ##############################################################################
# Status
# ##############################################################################
set(fields
    CMAKE_PROJECT_VERSION
    RCSW_BUILD_FOR
    RCSW_CONFIG_LIBTYPE
    RCSW_CONFIG_ER_PLUGIN
    RCSW_CONFIG_PTR_ALIGN
    RCSW_CONFIG_NOALLOC
    RCSW_CONFIG_ZALLOC
    RCSW_CONFIG_TOOL_NO_GRIND
    RCSW_CONFIG_NO_STDIO
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
    RCSW_CONFIG_STDIO_PRINTF_CHECK_NULL)

libra_config_summary_prepare_fields("${fields}")

function(rcsw_message type msg)
  message(${type} "[RCSW] ${msg}")
endfunction()

message(
  "${BoldBlue}--------------------------------------------------------------------------------"
)
message("${BoldBlue}                           RCSW Configuration Summary")
message(
  "${BoldBlue}--------------------------------------------------------------------------------"
)

rcsw_message(
  STATUS
  "Version                                       : ${ColorBold}${EMIT_CMAKE_PROJECT_VERSION}${ColorReset} [CMAKE_PROJECT_VERSION]"
)
rcsw_message(
  STATUS
  "Building for                                  : ${ColorBold}${EMIT_RCSW_BUILD_FOR}${ColorReset} [RCSW_BUILD_FOR={POSIX,BAREMETAL}]"
)
rcsw_message(
  STATUS
  "Library type                                  : ${ColorBold}${EMIT_RCSW_CONFIG_LIBTYPE}${ColorReset} [RCSW_CONFIG_LIBTYPE={STATIC,SHARED}]"
)
rcsw_message(
  STATUS
  "Event reporting plugin                        : ${ColorBold}${EMIT_RCSW_CONFIG_ER_PLUGIN}${ColorReset} [RCSW_CONFIG_ER_PLUGIN={ZLOG,LOG4CL,SIMPLE}]"
)
rcsw_message(
  STATUS
  "Disable dynamic memory allocation             : ${ColorBold}${EMIT_RCSW_CONFIG_NOALLOC}${ColorReset} [RCSW_CONFIG_NOALLOC]"
)
rcsw_message(
  STATUS
  "Always zero alloc'd memory before use         : ${ColorBold}${EMIT_RCSW_CONFIG_ZALLOC}${ColorReset} [RCSW_CONFIG_ZALLOC]"
)
rcsw_message(
  STATUS
  "Data pointer alignment                        : ${ColorBold}${EMIT_RCSW_CONFIG_PTR_ALIGN}${ColorReset} [RCSW_CONFIG_PTR_ALIGN={1,2,4}]"
)
rcsw_message(
  STATUS
  "Compile out RCSW_GRIND_XX() macros            : ${ColorBold}${EMIT_RCSW_CONFIG_TOOL_NO_GRIND}${ColorReset} [RCSW_CONFIG_TOOL_NO_GRIND]"
)

if(NOT ${RCSW_CONFIG_NO_STDIO})
  rcsw_message(
    STATUS
    "Skip building STDIO module                    : ${ColorBold}${EMIT_RCSW_CONFIG_NO_STDIO}${ColorReset} [RCSW_CONFIG_NO_STDIO]"
  )
  rcsw_message(
    STATUS
    "stdio getchar() function                      : ${ColorBold}${EMIT_RCSW_CONFIG_STDIO_GETCHAR}${ColorReset} [RCSW_CONFIG_STDIO_GETCHAR]"
  )
  rcsw_message(
    STATUS
    "stdio putchar() function                      : ${ColorBold}${EMIT_RCSW_CONFIG_STDIO_PUTCHAR}${ColorReset} [RCSW_CONFIG_STDIO_PUTCHAR]"
  )
  rcsw_message(
    STATUS
    "stdio math taylor expansion terms             : ${ColorBold}${EMIT_RCSW_CONFIG_STDIO_MATH_LOG10_TERMS}${ColorReset} [RCSW_CONFIG_STDIO_MATH_LOG10_TERMS]"
  )
  rcsw_message(
    STATUS
    "stdio printf() buffer size                    : ${ColorBold}${EMIT_RCSW_CONFIG_STDIO_PRINTF_BUFSIZE}${ColorReset} [RCSW_CONFIG_STDIO_PRINTF_BUFSIZE]"
  )
  rcsw_message(
    STATUS
    "stdio printf() support for decimals           : ${ColorBold}${EMIT_RCSW_CONFIG_STDIO_PRINTF_WITH_DEC}${ColorReset} [RCSW_CONFIG_STDIO_PRINTF_WITH_DEC]"
  )
  rcsw_message(
    STATUS
    "stdio printf() support for exponentials       : ${ColorBold}${EMIT_RCSW_CONFIG_STDIO_PRINTF_WITH_EXP}${ColorReset} [RCSW_CONFIG_STDIO_PRINTF_WITH_EXP]"
  )
  rcsw_message(
    STATUS
    "stdio printf() support for writeback          : ${ColorBold}${EMIT_RCSW_CONFIG_STDIO_PRINTF_WITH_WRITEBACK}${ColorReset} [RCSW_CONFIG_STDIO_PRINTF_WITH_WRITEBACK]"
  )
  rcsw_message(
    STATUS
    "stdio printf() default float precision        : ${ColorBold}${EMIT_RCSW_CONFIG_STDIO_PRINTF_DEFAULT_FLOAT_PREC}${ColorReset} [RCSW_CONFIG_STDIO_PRINTF_DEFAULT_FLOAT_PREC]"
  )
  rcsw_message(
    STATUS
    "stdio printf() decimal -> exp digit threshold : ${ColorBold}${EMIT_RCSW_CONFIG_STDIO_PRINTF_EXP_DIGIT_THRESH}${ColorReset} [RCSW_CONFIG_STDIO_PRINTF_EXP_DIGIT_THRESH]"
  )
  rcsw_message(
    STATUS
    "stdio printf() support for long long          : ${ColorBold}${EMIT_RCSW_CONFIG_STDIO_PRINTF_WITH_LL}${ColorReset} [RCSW_CONFIG_STDIO_PRINTF_WITH_LL]"
  )
  rcsw_message(
    STATUS
    "stdio printf() fmt string safety check        : ${ColorBold}${EMIT_RCSW_CONFIG_STDIO_PRINTF_CHECK_NULL}${ColorReset} [RCSW_CONFIG_STDIO_PRINTF_CHECK_NULL]"
  )
else()
  rcsw_message(
    STATUS
    "Skip building STDIO module                    : ${ColorBold}${EMIT_RCSW_CONFIG_NO_STDIO}${ColorReset} [RCSW_CONFIG_NO_STDIO]"
  )
endif()

message(
  "${BoldBlue}--------------------------------------------------------------------------------"
)
