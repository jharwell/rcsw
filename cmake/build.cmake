# ##############################################################################
# User-facing options and cache variables
# ##############################################################################
set(RCSW_BUILD_FOR
    "POSIX"
    CACHE STRING "Target platform. One of: POSIX, BAREMETAL")
set_property(CACHE RCSW_BUILD_FOR PROPERTY STRINGS POSIX BAREMETAL)

# Defaults to BUILD_SHARED_LIBS, but an explicit -DRCSW_CONFIG_LIBTYPE wins.
if(BUILD_SHARED_LIBS)
  set(_rcsw_default_libtype SHARED)
else()
  set(_rcsw_default_libtype STATIC)
endif()
set(RCSW_CONFIG_LIBTYPE
    ${_rcsw_default_libtype}
    CACHE STRING "Library type. One of: STATIC, SHARED.")
unset(_rcsw_default_libtype)
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
       YES)

# The component options are in components.cmake: which ones are available
# depends on the platform settings below.

# ##############################################################################
# Platform-specific settings
#
# Runs before components.cmake: the event reporting plugin forced here decides
# whether stdio is part of core.
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

    if(NOT "${RCSW_CONFIG_ER_PLUGIN}" STREQUAL "SIMPLE")
      rcsw_message(STATUS "Overriding RCSW_CONFIG_ER_PLUGIN to SIMPLE \
(baremetal + no stdlib does not support ${RCSW_CONFIG_ER_PLUGIN})")
    endif()
    set(RCSW_CONFIG_ER_PLUGIN
        SIMPLE
        CACHE STRING "" FORCE)
  endif()
else()
  rcsw_message(FATAL_ERROR "RCSW_BUILD_FOR must be one of: POSIX, BAREMETAL")
endif()

# Pointer alignment auto-detection. Must run before targets.cmake, so that
# RCSW_CONFIG_PTR_ALIGN is non-empty when compile definitions are applied.
if(NOT RCSW_CONFIG_PTR_ALIGN)
  if("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "^(x86_64|AMD64|aarch64|arm64)$")
    set(RCSW_CONFIG_PTR_ALIGN 8)
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
Set it explicitly via -DRCSW_CONFIG_PTR_ALIGN=<1|2|4|8>.")
endif()

if(NOT RCSW_CONFIG_LIBTYPE MATCHES "^(STATIC|SHARED)$")
  rcsw_message(FATAL_ERROR "RCSW_CONFIG_LIBTYPE must be one of: STATIC, SHARED")
endif()
