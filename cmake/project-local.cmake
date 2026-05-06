# ##############################################################################
# General Configuration
# ##############################################################################
set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 2)
set(PROJECT_VERSION_PATCH 28)
set(rcsw_VERSION
    "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}"
)

if(NOT RCSW_BUILD_FOR)
  set(RCSW_BUILD_FOR "POSIX")
endif()

if(NOT RCSW_CONFIG_LIBTYPE)
  set(RCSW_CONFIG_LIBTYPE STATIC)
endif()

# Shared libraries don't make sense on bare-metal/freestranding targets
if("${RCSW_BUILD_FOR}" MATCHES "POSIX")
  # We might be linking with a shared library
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
elseif("${RCSW_BUILD_FOR}" MATCHES "BAREMETAL")
  set(RCSW_CONFIG_LIBTYPE STATIC)
  set(CMAKE_POSITION_INDEPENDENT_CODE OFF)

  if(${LIBRA_STDLIB} MATCHES "NONE")
    set(RCSW_CONFIG_NOALLOC YES)
    set(RCSW_CONFIG_ER_PLUGIN SIMPLE)
    if(${RCSW_WITHOUT_STDIO})
      message(FATAL_ERROR "RCSW requires RCSW_WITHOUT_STDIO=NO when
building for baremetal without the standard library.")
    endif()
  endif()
endif()

libra_configure_source_file(
  ${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/src/version/version.c.in
  ${CMAKE_CURRENT_BINARY_DIR}/src/version/version.c rcsw_components_SRC)

# ##############################################################################
# Memory
# ##############################################################################
# Are we allowed to allocate memory?
if(NOT RCSW_CONFIG_NOALLOC)
  set(RCSW_CONFIG_NOALLOC NO)
endif()

# Should all memory be zeroed before use (regardless of where it came from) ?
if(NOT RCSW_CONFIG_ZALLOC)
  set(RCSW_CONFIG_ZALLOC NO)
endif()

# ##############################################################################
# STDIO
# ##############################################################################
if(NOT RCSW_WITHOUT_STDIO)
  set(RCSW_WITHOUT_STDIO NO)
endif()

if(NOT ${RCSW_WITHOUT_STDIO})
  if(NOT RCSW_CONFIG_STDIO_PRINTF_BUFSIZE)
    set(RCSW_CONFIG_STDIO_PRINTF_BUFSIZE 32)
  endif()

  if(NOT RCSW_CONFIG_STDIO_PRINTF_WITH_DEC)
    set(RCSW_CONFIG_STDIO_PRINTF_WITH_DEC YES)
  endif()

  if(NOT RCSW_CONFIG_STDIO_PRINTF_WITH_EXP)
    set(RCSW_CONFIG_STDIO_PRINTF_WITH_EXP YES)
  endif()

  if(NOT RCSW_CONFIG_STDIO_PRINTF_WITH_WRITEBACK)
    set(RCSW_CONFIG_STDIO_PRINTF_WITH_WRITEBACK YES)
  endif()

  if(NOT RCSW_CONFIG_STDIO_PRINTF_DEFAULT_FLOAT_PREC)
    set(RCSW_CONFIG_STDIO_PRINTF_DEFAULT_FLOAT_PREC 6)
  endif()

  if(NOT RCSW_CONFIG_STDIO_PRINTF_EXP_DIGIT_THRESH)
    set(RCSW_CONFIG_STDIO_PRINTF_EXP_DIGIT_THRESH 9)
  endif()

  if(NOT RCSW_CONFIG_STDIO_PRINTF_WITH_LL)
    set(RCSW_CONFIG_STDIO_PRINTF_WITH_LL YES)
  endif()

  if(NOT RCSW_CONFIG_STDIO_MATH_LOG10_TERMS)
    set(RCSW_CONFIG_STDIO_MATH_LOG10_TERMS 4)
  endif()

  if(NOT RCSW_CONFIG_STDIO_PRINTF_CHECK_NULL)
    set(RCSW_CONFIG_STDIO_PRINTF_CHECK_NULL YES)
  endif()

  if(NOT RCSW_CONFIG_STDIO_PUTCHAR)
    set(RCSW_CONFIG_STDIO_PUTCHAR putchar)
  endif()

  if(NOT RCSW_CONFIG_STDIO_GETCHAR)
    set(RCSW_CONFIG_STDIO_GETCHAR getchar)
  endif()

endif()

# ##############################################################################
# Event Reporting
# ##############################################################################
if(NOT RCSW_CONFIG_ER_PLUGIN)
  set(RCSW_CONFIG_ER_PLUGIN LOG4CL)
endif()

# ##############################################################################
# Tools
# ##############################################################################
if(NOT RCSW_CONFIG_TOOL_NO_GRIND)
  set(RCSW_CONFIG_TOOL_NO_GRIND NO)
endif()

# ##############################################################################
# Misc.
# ##############################################################################
if(NOT RCSW_CONFIG_PTR_ALIGN)
  # We know x86 can handle 1 byte-aligned addresses; add other architectures
  # here as needed.
  if("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "x86_64")
    set(RCSW_CONFIG_PTR_ALIGN 4)
    # We know ARM can't handle 1 byte-aligned accesses from say a 4-byte aligned
    # pointer.
  elseif("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "arm")
    set(RCSW_CONFIG_PTR_ALIGN 1)
  else()
    # The fallback is the (possibly) less-efficient 1 byte alignment.
    set(RCSW_CONFIG_PTR_ALIGN 1)
    message(
      WARNING
        "Novel build target architecture '${CMAKE_SYSTEM_PROCESSOR}'--default to byte aligned data storage"
    )
  endif()
endif()

# ##############################################################################
# Components
# ##############################################################################
if("${RCSW_BUILD_FOR}" MATCHES "BAREMETAL")
  include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/baremetal.cmake)
  rcsw_baremetal_configure_components()
elseif("${RCSW_BUILD_FOR}" MATCHES "POSIX")
  include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/posix.cmake)
  rcsw_posix_configure_components()
else()
  message(FATAL_ERROR "AL target must be {POSIX,BAREMETAL}")
endif()

libra_requested_components_check(rcsw)
# ##############################################################################
# Libraries
# ##############################################################################
# Create the source for the SINGLE library to build by combining the source of
# the selected components
foreach(component ${rcsw_FIND_COMPONENTS})
  if(${rcsw_${component}_FOUND})
    list(APPEND rcsw_components_SRC ${rcsw_} ${rcsw_${component}_SRC})
  endif()
endforeach()

libra_add_library(${PROJECT_NAME} ${RCSW_CONFIG_LIBTYPE} ${rcsw_components_SRC})
# Available whether installed or subdirectory via CPM
if(NOT TARGET rcsw::rcsw)
  add_library(rcsw::rcsw ALIAS rcsw)
endif()

# Setting this results in TWO files being installed: the actual library with the
# version embedded, and a symlink to the actual library with the same name sans
# the embedded version (if rcsw is built as a shared library).
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${rcsw_VERSION}
                                                 SOVERSION ${rcsw_VERSION})

# ##############################################################################
# Compile Definitions
# ##############################################################################
target_compile_definitions(
  ${PROJECT_NAME}
  PRIVATE RCSW_CONFIG_ER_PLUGIN=RCSW_ER_PLUGIN_${RCSW_CONFIG_ER_PLUGIN})
target_compile_definitions(
  ${PROJECT_NAME} PUBLIC RCSW_CONFIG_AL_TARGET=RCSW_AL_TARGET_${RCSW_BUILD_FOR})

set(RCSW_ONOFF_CONFIG
    RCSW_CONFIG_STDIO_PRINTF_WITH_DEC
    RCSW_CONFIG_STDIO_PRINTF_WITH_EXP
    RCSW_CONFIG_STDIO_PRINTF_WITH_WRITEBACK
    RCSW_CONFIG_STDIO_PRINTF_WITH_LL
    RCSW_CONFIG_STDIO_PRINTF_CHECK_NULL
    RCSW_CONFIG_TOOL_NO_GRIND
    RCSW_CONFIG_NOALLOC
    RCSW_CONFIG_ZALLOC)

foreach(config ${RCSW_ONOFF_CONFIG})
  if(${config})
    target_compile_definitions(${PROJECT_NAME} PRIVATE ${config})
  endif()
endforeach()

set(RCSW_VALUE_CONFIG_PUBLIC RCSW_CONFIG_STDIO_PUTCHAR
                             RCSW_CONFIG_STDIO_GETCHAR RCSW_CONFIG_PTR_ALIGN)
set(RCSW_VALUE_CONFIG_PRIVATE
    RCSW_CONFIG_STDIO_PRINTF_BUFSIZE
    RCSW_CONFIG_STDIO_PRINTF_DEFAULT_FLOAT_PREC
    RCSW_CONFIG_STDIO_PRINTF_EXP_DIGIT_THRESH
    RCSW_CONFIG_STDIO_MATH_LOG10_TERMS)
foreach(config ${RCSW_VALUE_CONFIG_PUBLIC})
  if(${config})
    target_compile_definitions(${PROJECT_NAME} PUBLIC ${config}=${${config}})
  endif()
endforeach()
foreach(config ${RCSW_VALUE_CONFIG_PRIVATE})
  if(${config})
    target_compile_definitions(${PROJECT_NAME} PRIVATE ${config}=${${config}})
  endif()
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
# Link Libraries
# ##############################################################################
target_link_libraries(${PROJECT_NAME} PUBLIC pthread dl m)

if("${RCSW_CONFIG_ER_PLUGIN}" STREQUAL "ZLOG")
  target_include_directories(
    ${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${LIBRA_DEPS_PREFIX}/include>)
  target_link_libraries(${PROJECT_NAME} PUBLIC zlog)
  target_link_directories(${PROJECT_NAME} PUBLIC ${LIBRA_DEPS_PREFIX}/lib)
endif()

# ##############################################################################
# Installation and Deployment
# ##############################################################################
# Installation, but only on POSIX/linux platforms. For embedded platforms it
# doesn't make sense.
if("${RCSW_BUILD_FOR}" MATCHES "POSIX")
  libra_configure_exports(${PROJECT_NAME})
  libra_register_target_for_install(${PROJECT_NAME})
  libra_register_headers_for_install(include/${PROJECT_NAME})

  # Deployment
  if(NOT CPACK_PACKAGE_NAME)
    set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
  endif()

  libra_register_copyright_for_install(${PROJECT_NAME}
                                       ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE)

  set(RCSW_PKG_SUMMARY
      "Collection of Reusable C SoftWare (RCSW) modules for embedded programming"
  )

  set(RCSW_PKG_DESCRIPTION
      "Collection of reusable C software modules for embedded programming,
styled after the C++ STL. Features:

* Many data structures (lists, queues, trees, hash tables)
* Publisher-subscriber system
* Plugin-based event reporting framework
* Simple stdlib replacement for bare-metal applications

This is a ${RCSW_CONFIG_LIBTYPE} library, built for ${RCSW_BUILD_FOR}:
  * RCSW_CONFIG_ER_PLUGIN=${RCSW_CONFIG_ER_PLUGIN}
  * RCSW_CONFIG_PTR_ALIGN=${RCSW_CONFIG_PTR_ALIGN}
  * RCSW_CONFIG_NOALLOC=${RCSW_CONFIG_NOALLOC}
  * RCSW_CONFIG_ZALLOC=${RCSW_CONFIG_ZALLOC}
  * RCSW_CONFIG_TOOL_NO_GRIND=${RCSW_CONFIG_TOOL_NO_GRIND}
  * RCSW_WITHOUT_STDIO=${RCSW_WITHOUT_STDIO}
  * RCSW_CONFIG_STDIO_GETCHAR=${RCSW_CONFIG_STDIO_GETCHAR}
  * RCSW_CONFIG_STDIO_PUTCHAR=${RCSW_CONFIG_STDIO_PUTCHAR}
  * RCSW_CONFIG_STDIO_MATH_LOG10_TERMS=${RCSW_CONFIG_STDIO_MATH_LOG10_TERMS}
  * RCSW_CONFIG_STDIO_PRINTF_BUFSIZE=${RCSW_CONFIG_STDIO_PRINTF_BUFSIZE}
  * RCSW_CONFIG_STDIO_PRINTF_WITH_DEC=${RCSW_CONFIG_STDIO_PRINTF_WITH_DEC}
  * RCSW_CONFIG_STDIO_PRINTF_WITH_EXP=${RCSW_CONFIG_STDIO_PRINTF_WITH_EXP}
  * RCSW_CONFIG_STDIO_PRINTF_WITH_WRITEBACK=${RCSW_CONFIG_STDIO_PRINTF_WITH_WRITEBACK}
  * RCSW_CONFIG_STDIO_PRINTF_DEFAULT_FLOAT_PREC=${RCSW_CONFIG_STDIO_PRINTF_DEFAULT_FLOAT_PREC}
  * RCSW_CONFIG_STDIO_PRINTF_EXP_DIGIT_THRESH=${RCSW_CONFIG_STDIO_PRINTF_EXP_DIGIT_THRESH}
  * RCSW_CONFIG_STDIO_PRINTF_WITH_LL=${RCSW_CONFIG_STDIO_PRINTF_WITH_LL}
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
    rcsw_VERSION
    RCSW_BUILD_FOR
    RCSW_CONFIG_LIBTYPE
    RCSW_CONFIG_ER_PLUGIN
    RCSW_CONFIG_PTR_ALIGN
    RCSW_CONFIG_NOALLOC
    RCSW_CONFIG_ZALLOC
    RCSW_CONFIG_TOOL_NO_GRIND
    RCSW_WITHOUT_STDIO
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
  "Version                                       : ${ColorBold}${EMIT_rcsw_VERSION}${ColorReset} [rcsw_VERSION]"
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

if(NOT ${RCSW_WITHOUT_STDIO})
  rcsw_message(
    STATUS
    "Skip building STDIO module                    : ${ColorBold}${EMIT_RCSW_WITHOUT_STDIO}${ColorReset} [RCSW_WITHOUT_STDIO]"
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
    "Skip building STDIO module                    : ${ColorBold}${EMIT_RCSW_WITHOUT_STDIO}${ColorReset} [RCSW_WITHOUT_STDIO]"
  )
endif()

rcsw_message(
  STATUS
  "Compile out RCSW_GRIND_XX() macros            : ${ColorBold}${EMIT_RCSW_CONFIG_TOOL_NO_GRIND}${ColorReset} [RCSW_CONFIG_TOOL_NO_GRIND]"
)

message(
  "${BoldBlue}--------------------------------------------------------------------------------"
)
