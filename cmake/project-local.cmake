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

# 2026-04-29 [JRH]: We always build as static lib to avoid annoying issues with
# putchar_() needing to be exported from RCSW.
cpmaddpackage(
  NAME
  printf
  GITHUB_REPOSITORY
  eyalroz/printf
  VERSION
  6.3.0
  OPTIONS
  "BUILD_SHARED_LIBS OFF"
  "PRINTF_SUPPORT_DECIMAL_SPECIFIERS ON"
  "PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS ON"
  "PRINTF_SUPPORT_WRITEBACK_SPECIFIER ON"
  "PRINTF_SUPPORT_LONG_LONG ON"
  "PRINTF_NTOA_BUFFER_SIZE 32"
  "PRINTF_DEFAULT_FLOAT_PRECISION 6"
  "PRINTF_MAX_INTEGRAL_DIGITS_FOR_DECIMAL 9"
  "PRINTF_CHECK_FOR_NUL_IN_FORMAT_SPECIFIER ON")

# Override the printf target's include path to avoid collision with rcsw/include
# and system headers
set_target_properties(printf PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
                                        "${printf_SOURCE_DIR}/src")

file(WRITE "${CMAKE_BINARY_DIR}/include/eyalroz/printf.h"
     "#include \"${printf_SOURCE_DIR}/src/printf/printf.h\"\n")

set(LIBRA_TEST_HARNESS_LIBS Catch2::Catch2WithMain)

function(rcsw_message type msg)
  message(${type} "[RCSW] ${msg}")
endfunction()

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

set(RCSW_CONFIG_STDIO_PUTCHAR
    putchar
    CACHE STRING "stdio putchar() replacement function")
set(RCSW_CONFIG_STDIO_GETCHAR
    getchar
    CACHE STRING "stdio getchar() replacement function")

option(RCSW_CONFIG_NOALLOC "Disable dynamic memory allocation" OFF)
option(RCSW_CONFIG_ZALLOC "Zero all allocated memory before use" OFF)
option(RCSW_CONFIG_TOOL_NO_GRIND "Compile out RCSW_GRIND_XX() macros" OFF)

option(RCSW_CONFIG_BUILD_MONOLITHIC
       "Build the monolithic rcsw library in addition to component libraries"
       OFF)

# These aren't really optional disabling them will cause bad things to happen in
# the build, but it is far more convenient when emitting the summary if they are
# here.
option(RCSW_CONFIG_CORE "Build RCSW core component" YES)
option(RCSW_CONFIG_AL "Build abstraction layer component" YES)

option(RCSW_CONFIG_ALGORITHM "Build the algorithm component" ON)
option(RCSW_CONFIG_DS "Build the data structures component" ON)
option(RCSW_CONFIG_ER "Build the event reporting component" ON)
option(RCSW_CONFIG_MULTIPROCESS "Build the multiprocess component " YES)
option(RCSW_CONFIG_MULTITHREAD "Build the multithread component" ON)
option(RCSW_CONFIG_STDIO "Build the STDIO component" ON)
option(RCSW_CONFIG_SWBUS "Build the swbus component" ON)
option(RCSW_CONFIG_TOOL "Build the tool component" ON)
option(RCSW_CONFIG_UTILS "Build the utils component" ON)

# ##############################################################################
# Components
# ##############################################################################
set(_OPTIONAL_COMPONENTS
    algorithm
    ds
    er
    multiprocess
    multithread
    stdio
    swbus
    tool
    utils)
if("${RCSW_BUILD_FOR}" MATCHES "BAREMETAL")
  list(
    FILTER
    _OPTIONAL_COMPONENTS
    EXCLUDE
    REGEX
    "multithread|multiprocess|swbus")
  list(
    FILTER
    ${PROJECT_NAME}_C_SRC
    EXCLUDE
    REGEX
    "src/multithread|src/multiprocess")
  if(LIBRA_STDLIB MATCHES "NONE")
    # Without a stdlib, er and ds cannot be built
    list(
      FILTER
      _OPTIONAL_COMPONENTS
      EXCLUDE
      REGEX
      "^er$|^ds$")
    list(
      FILTER
      ${PROJECT_NAME}_C_SRC
      EXCLUDE
      REGEX
      "src/er|src/ds")
  endif()
elseif("${RCSW_BUILD_FOR}" MATCHES "POSIX")
  list(
    FILTER
    _OPTIONAL_COMPONENTS
    EXCLUDE
    REGEX
    "baremetal")
  list(
    FILTER
    ${PROJECT_NAME}_C_SRC
    EXCLUDE
    REGEX
    "baremetal")
  if(NOT RCSW_CONFIG_MULTIPROCESS)
    list(
      FILTER
      ${PROJECT_NAME}_C_SRC
      EXCLUDE
      REGEX
      "multiprocess")
  endif()
else()
  message(FATAL_ERROR "RCSW_BUILD_FOR must be one of: POSIX, BAREMETAL")
endif()

# Build whatever survived the platform filter and has its option ON.
set(_CONFIGURED_COMPONENTS al core)
foreach(_component IN LISTS _OPTIONAL_COMPONENTS)
  string(TOUPPER "${_component}" _opt)
  if(RCSW_CONFIG_${_opt})
    list(APPEND _CONFIGURED_COMPONENTS ${_component})
  endif()
endforeach()

# Guard: warn loudly if there is genuinely nothing useful to build.
if(NOT RCSW_CONFIG_BUILD_MONOLITHIC)
  set(_optional_configured FALSE)
  foreach(_component IN LISTS _CONFIGURED_COMPONENTS)
    if(NOT (_component STREQUAL "al" OR _component STREQUAL "core"))
      set(_optional_configured TRUE)
      break()
    endif()
  endforeach()
  if(NOT _optional_configured)
    message(
      WARNING
        "RCSW_CONFIG_BUILD_MONOLITHIC is OFF and no optional components are \
enabled -- only the al and core stubs will be built. Enable at least one \
RCSW_CONFIG_<COMPONENT> option or set RCSW_CONFIG_BUILD_MONOLITHIC=ON.")
  endif()
  unset(_optional_configured)
endif()

# ##############################################################################
# Platform-specific overrides
# ##############################################################################
if("${RCSW_BUILD_FOR}" MATCHES "POSIX")
  rcsw_message(STATUS "Building for POSIX")
elseif("${RCSW_BUILD_FOR}" MATCHES "BAREMETAL")
  rcsw_message(STATUS "Building for baremetal")

  # Shared libraries don't make sense on bare-metal/freestanding targets
  set(RCSW_CONFIG_LIBTYPE
      STATIC
      CACHE STRING "" FORCE)
  set(CMAKE_POSITION_INDEPENDENT_CODE OFF)

  if(${LIBRA_STDLIB} MATCHES "NONE")
    set(RCSW_CONFIG_NOALLOC YES)

    # FIX: emit a visible message before silently overriding a user-set value
    if(NOT "${RCSW_CONFIG_ER_PLUGIN}" STREQUAL "SIMPLE")
      rcsw_message(STATUS "Overriding RCSW_CONFIG_ER_PLUGIN to SIMPLE \
(baremetal + no stdlib does not support ${RCSW_CONFIG_ER_PLUGIN})")
    endif()
    set(RCSW_CONFIG_ER_PLUGIN
        SIMPLE
        CACHE STRING "" FORCE)

    # FIX: use lowercase "stdio" to match the list contents
    if(NOT "stdio" IN_LIST _CONFIGURED_COMPONENTS)
      message(
        FATAL_ERROR
          "RCSW requires the stdio component when building for baremetal \
without the standard library. Enable RCSW_CONFIG_STDIO.")
    endif()
  endif()
endif()

# Pointer alignment auto-detection. NOTE: This must run before the library
# targets section so that RCSW_CONFIG_PTR_ALIGN is non-empty when compile
# definitions are applied. version.c is appended to ${PROJECT_NAME}_C_SRC by
# libra_configure_source_file below; it is intentionally added after the
# platform source filters above since it is platform-independent and should
# never be excluded.
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

# Sanity check: auto-detection or an explicit value must have produced something
# non-empty before we proceed to bake it into compile definitions.
if(NOT RCSW_CONFIG_PTR_ALIGN)
  message(FATAL_ERROR "RCSW_CONFIG_PTR_ALIGN is empty after auto-detection. \
Set it explicitly via -DRCSW_CONFIG_PTR_ALIGN=<1|2|4>.")
endif()

# ##############################################################################
# Library targets
# ##############################################################################

# Generate version.c and append it to the source list unconditionally. This must
# run before any target is created (monolithic or stub) so that: (a) the
# generated file exists when the build system is invoked, and (b) the stub rcsw
# target (when RCSW_CONFIG_BUILD_MONOLITHIC is OFF) does not silently inherit
# version.c via a later side-effecting call inside the component loop, only to
# then compile it without any definitions.
libra_configure_source_file(
  ${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/src/version/version.c.in
  ${CMAKE_CURRENT_BINARY_DIR}/src/version/version.c ${PROJECT_NAME}_C_SRC)

# minimon is baremetal-only; exclude it before any targets are created so the
# filtered source list is consistent for both monolithic and component paths.
if("${RCSW_BUILD_FOR}" MATCHES "POSIX")
  list(
    FILTER
    ${PROJECT_NAME}_C_SRC
    EXCLUDE
    REGEX
    "src/tool/minimon")
endif()

# Monolithic library (optional)
if(RCSW_CONFIG_BUILD_MONOLITHIC)
  libra_add_library(${PROJECT_NAME} ${RCSW_CONFIG_LIBTYPE}
                    ${${PROJECT_NAME}_C_SRC})
  if(NOT TARGET rcsw::rcsw)
    add_library(rcsw::rcsw ALIAS rcsw)
  endif()
  set_target_properties(
    ${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION}
                               SOVERSION ${PROJECT_VERSION_MAJOR})
else()
  # A valid parent target is always required by LIBRA's component machinery even
  # when the monolithic library is not being installed/exported.
  libra_add_library(${PROJECT_NAME} ${RCSW_CONFIG_LIBTYPE})
endif()

# Component libraries (always built regardless of monolithic setting)
set(_COMPONENT_LIBS)
foreach(_component IN LISTS _CONFIGURED_COMPONENTS)
  if(_component STREQUAL "al")
    string(TOLOWER "${RCSW_BUILD_FOR}" _platform)
    libra_add_component_library(
      TARGET
      ${PROJECT_NAME}
      COMPONENT
      ${_component}
      SOURCES
      ${${PROJECT_NAME}_C_SRC}
      REGEX
      "src/al/${_platform}")
  elseif(_component STREQUAL "core")
    libra_add_component_library(
      TARGET
      ${PROJECT_NAME}
      COMPONENT
      ${_component}
      SOURCES
      ${${PROJECT_NAME}_C_SRC}
      REGEX
      "src/core")
  elseif(_component STREQUAL tool AND "${RCSW_BUILD_FOR}" MATCHES "POSIX")

    # minimon is baremetal-only, so exclude it
    libra_add_component_library(
      TARGET
      ${PROJECT_NAME}
      COMPONENT
      ${_component}
      SOURCES
      ${${PROJECT_NAME}_C_SRC}
      REGEX
      "src/tool/grind")
  else()
    libra_add_component_library(
      TARGET
      ${PROJECT_NAME}
      COMPONENT
      ${_component}
      SOURCES
      ${${PROJECT_NAME}_C_SRC}
      REGEX
      "src/${_component}")
  endif()
  list(APPEND _COMPONENT_LIBS ${PROJECT_NAME}_${_component})
endforeach()

# --- Namespace aliases for all component targets ------------------------------
foreach(_component IN LISTS _CONFIGURED_COMPONENTS)
  add_library(rcsw::${_component} ALIAS ${PROJECT_NAME}_${_component})
endforeach()

# ##############################################################################
# Compile definitions
# ##############################################################################
function(_rcsw_apply_compile_defs target)
  target_compile_definitions(
    ${target}
    PRIVATE RCSW_CONFIG_ER_PLUGIN=RCSW_ER_PLUGIN_${RCSW_CONFIG_ER_PLUGIN}
    PUBLIC RCSW_CONFIG_PLATFORM=RCSW_CONFIG_PLATFORM_${RCSW_BUILD_FOR})

  foreach(_config IN ITEMS RCSW_CONFIG_TOOL_NO_GRIND RCSW_CONFIG_NOALLOC
                           RCSW_CONFIG_ZALLOC)
    if(${_config})
      target_compile_definitions(${target} PRIVATE ${_config})
    endif()
  endforeach()

  foreach(_config IN ITEMS RCSW_CONFIG_STDIO_PUTCHAR RCSW_CONFIG_STDIO_GETCHAR
                           RCSW_CONFIG_PTR_ALIGN)
    target_compile_definitions(${target} PUBLIC ${_config}=${${_config}})
  endforeach()

  get_target_property(_ttype ${target} TYPE)
  if(_ttype STREQUAL SHARED_LIBRARY)
    set_target_properties(${target} PROPERTIES C_VISIBILITY_PRESET hidden)
  endif()
endfunction()

foreach(_lib IN LISTS _COMPONENT_LIBS)
  _rcsw_apply_compile_defs(${_lib})
endforeach()
# The parent target always exists (monolithic or stub) and always compiles
# version.c, so it always needs definitions regardless of
# RCSW_CONFIG_BUILD_MONOLITHIC.
_rcsw_apply_compile_defs(${PROJECT_NAME})

# ##############################################################################
# Include directories
# ##############################################################################
function(_rcsw_apply_includes target)
  target_include_directories(
    ${target} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                     $<INSTALL_INTERFACE:include>)
endfunction()

foreach(_lib IN LISTS _COMPONENT_LIBS)
  _rcsw_apply_includes(${_lib})
endforeach()
# Same reasoning as compile defs: the parent target always needs includes.
_rcsw_apply_includes(${PROJECT_NAME})

# Some components need the generated eyalroz/printf.h wrapper
if(RCSW_CONFIG_STDIO)
  target_include_directories(${PROJECT_NAME}_stdio
                             PRIVATE ${CMAKE_BINARY_DIR}/include)
  if(RCSW_CONFIG_TOOL)
    target_include_directories(${PROJECT_NAME}_tool
                               PRIVATE ${CMAKE_BINARY_DIR}/include)
  endif()
  if(RCSW_CONFIG_BUILD_MONOLITHIC)
    target_include_directories(${PROJECT_NAME}
                               PRIVATE ${CMAKE_BINARY_DIR}/include)
  endif()
endif()

# ##############################################################################
# Link libraries
# ##############################################################################
if("${RCSW_BUILD_FOR}" MATCHES "POSIX")
  foreach(_lib IN LISTS _COMPONENT_LIBS)
    target_link_libraries(${_lib} PUBLIC pthread dl m)
  endforeach()
  if(RCSW_CONFIG_BUILD_MONOLITHIC)
    target_link_libraries(${PROJECT_NAME} PUBLIC pthread dl m)
  endif()
endif()

if("${RCSW_CONFIG_ER_PLUGIN}" STREQUAL "ZLOG")
  if("er" IN_LIST _CONFIGURED_COMPONENTS)
    target_link_libraries(${PROJECT_NAME}_er PUBLIC zlog)
  endif()
  if(RCSW_CONFIG_BUILD_MONOLITHIC)
    target_link_libraries(${PROJECT_NAME} PUBLIC zlog)
  endif()
endif()
if(RCSW_CONFIG_STDIO)
  target_link_libraries(${PROJECT_NAME}_stdio PRIVATE printf)
  if(RCSW_CONFIG_BUILD_MONOLITHIC)
    target_link_libraries(${PROJECT_NAME} PRIVATE printf)
  endif()
endif()

# ##############################################################################
# Installation and deployment
# ##############################################################################
# Only applies to POSIX/Linux; embedding targets don't install.
if("${RCSW_BUILD_FOR}" MATCHES "POSIX")
  libra_configure_exports(${PROJECT_NAME})

  # Install every component library
  foreach(_component IN LISTS _CONFIGURED_COMPONENTS)
    libra_install_target(${PROJECT_NAME}_${_component} INCLUDE_DIR
                         include/${PROJECT_NAME}/${_component})
  endforeach()

  # Install monolithic library only if built
  if(RCSW_CONFIG_BUILD_MONOLITHIC)
    libra_install_target(${PROJECT_NAME} INCLUDE_DIR include/${PROJECT_NAME})
  endif()

  if(RCSW_CONFIG_BUILD_MONOLITHIC)
    libra_install_copyright(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE)
  else()
    libra_install_copyright(${PROJECT_NAME}_core
                            ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE)
  endif()

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
  * RCSW_CONFIG_BUILD_MONOLITHIC=${RCSW_CONFIG_BUILD_MONOLITHIC}\n\
  * RCSW_CONFIG_ER_PLUGIN=${RCSW_CONFIG_ER_PLUGIN}\n\
  * RCSW_CONFIG_PTR_ALIGN=${RCSW_CONFIG_PTR_ALIGN}\n\
  * RCSW_CONFIG_NOALLOC=${RCSW_CONFIG_NOALLOC}\n\
  * RCSW_CONFIG_ZALLOC=${RCSW_CONFIG_ZALLOC}\n\
  * RCSW_CONFIG_TOOL_NO_GRIND=${RCSW_CONFIG_TOOL_NO_GRIND}\n\
  * RCSW_CONFIG_AL=${RCSW_CONFIG_AL}\n\
  * RCSW_CONFIG_ALGORITHM=${RCSW_CONFIG_ALGORITHM}\n\
  * RCSW_CONFIG_CORE=${RCSW_CONFIG_CORE}\n\
  * RCSW_CONFIG_DS=${RCSW_CONFIG_DS}\n\
  * RCSW_CONFIG_ER=${RCSW_CONFIG_ER}\n\
  * RCSW_CONFIG_MULTIPROCESS=${RCSW_CONFIG_MULTIPROCESS}\n\
  * RCSW_CONFIG_MULTITHREAD=${RCSW_CONFIG_MULTITHREAD}\n\
  * RCSW_CONFIG_STDIO=${RCSW_CONFIG_STDIO}\n\
  * RCSW_CONFIG_STDIO_GETCHAR=${RCSW_CONFIG_STDIO_GETCHAR}\n\
  * RCSW_CONFIG_STDIO_PUTCHAR=${RCSW_CONFIG_STDIO_PUTCHAR}\n\
  * RCSW_CONFIG_TOOL=${RCSW_CONFIG_TOOL}\n\
  * RCSW_CONFIG_UTILS=${RCSW_CONFIG_UTILS}")

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
set(_summary_fields
    CMAKE_PROJECT_VERSION
    RCSW_BUILD_FOR
    RCSW_CONFIG_LIBTYPE
    RCSW_CONFIG_BUILD_MONOLITHIC
    RCSW_CONFIG_ER_PLUGIN
    RCSW_CONFIG_PTR_ALIGN
    RCSW_CONFIG_NOALLOC
    RCSW_CONFIG_ZALLOC
    RCSW_CONFIG_TOOL_NO_GRIND
    RCSW_CONFIG_STDIO_PUTCHAR
    RCSW_CONFIG_STDIO_GETCHAR)

libra_config_summary_prepare_fields("${_summary_fields}")

# Column width for the label field (left of the colon)
set(_W 46)

message(
  "--------------------------------------------------------------------------------"
)
message("                           RCSW Configuration Summary")
message(
  "--------------------------------------------------------------------------------"
)

# Helper to emit a padded summary line: _rcsw_summary_line(label value hint)
function(_rcsw_summary_line label value hint)
  string(LENGTH "${label}" _len)
  math(EXPR _pad "${_W} - ${_len}")
  string(REPEAT " " ${_pad} _spaces)
  rcsw_message(STATUS "${label}${_spaces}: ${value} [${hint}]")
endfunction()

_rcsw_summary_line("Version" "${EMIT_CMAKE_PROJECT_VERSION}"
                   "CMAKE_PROJECT_VERSION")
_rcsw_summary_line("Building for" "${EMIT_RCSW_BUILD_FOR}"
                   "RCSW_BUILD_FOR={POSIX,BAREMETAL}")
_rcsw_summary_line("Library type" "${EMIT_RCSW_CONFIG_LIBTYPE}"
                   "RCSW_CONFIG_LIBTYPE={STATIC,SHARED}")
_rcsw_summary_line(
  "Build monolithic lib" "${EMIT_RCSW_CONFIG_BUILD_MONOLITHIC}"
  "RCSW_CONFIG_BUILD_MONOLITHIC")
_rcsw_summary_line("Event reporting plugin" "${EMIT_RCSW_CONFIG_ER_PLUGIN}"
                   "RCSW_CONFIG_ER_PLUGIN={ZLOG,LOG4CL,SIMPLE}")
_rcsw_summary_line("No dynamic memory allocation" "${EMIT_RCSW_CONFIG_NOALLOC}"
                   "RCSW_CONFIG_NOALLOC")
_rcsw_summary_line("Zero alloc'd memory" "${EMIT_RCSW_CONFIG_ZALLOC}"
                   "RCSW_CONFIG_ZALLOC")
_rcsw_summary_line("Pointer alignment" "${EMIT_RCSW_CONFIG_PTR_ALIGN}"
                   "RCSW_CONFIG_PTR_ALIGN={1,2,4}")
_rcsw_summary_line("No GRIND macros" "${EMIT_RCSW_CONFIG_TOOL_NO_GRIND}"
                   "RCSW_CONFIG_TOOL_NO_GRIND")
_rcsw_summary_line("stdio putchar()" "${EMIT_RCSW_CONFIG_STDIO_PUTCHAR}"
                   "RCSW_CONFIG_STDIO_PUTCHAR")
_rcsw_summary_line("stdio getchar()" "${EMIT_RCSW_CONFIG_STDIO_GETCHAR}"
                   "RCSW_CONFIG_STDIO_GETCHAR")

message(
  "--------------------------------------------------------------------------------"
)
message("                           Components")
message(
  "--------------------------------------------------------------------------------"
)

# Drive YES/NO from _CONFIGURED_COMPONENTS (post-platform-filter) rather than
# the raw option value, so platform-excluded components show NO even when their
# option is ON.
foreach(
  _component IN
  ITEMS core
        al
        algorithm
        ds
        er
        multiprocess
        multithread
        stdio
        tool
        utils)
  string(TOUPPER "${_component}" _opt)
  if("${_component}" IN_LIST _CONFIGURED_COMPONENTS)
    set(_built "YES")
  else()
    set(_built "NO ")
  endif()
  _rcsw_summary_line("Build component ${_component}" "${_built}"
                     "RCSW_CONFIG_${_opt}")
endforeach()

message(
  "--------------------------------------------------------------------------------"
)
