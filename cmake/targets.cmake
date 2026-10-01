# ##############################################################################
# Library targets
#
# The monolithic library (optional), one library per configured component, and
# their headers, compile definitions, include directories and links.
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

# minimon is baremetal-only.
if(_RCSW_POSIX)
  list(
    FILTER
    ${PROJECT_NAME}_C_SRC
    EXCLUDE
    REGEX
    "src/tool/minimon")
endif()

# The directories under src/ and include/rcsw/ each component is made of.
set(_RCSW_PARTS_core ${_RCSW_CORE_PARTS})
foreach(_component IN LISTS RCSW_OPTIONAL_COMPONENTS)
  set(_RCSW_PARTS_${_component} ${_component})
endforeach()

set(_RCSW_ALL_PARTS)
foreach(_component IN LISTS _CONFIGURED_COMPONENTS)
  list(APPEND _RCSW_ALL_PARTS ${_RCSW_PARTS_${_component}})
endforeach()

# _rcsw_sources_regex(OUT PART...)
#
# Set OUT to a regex matching the sources of PART... . al's sources are split by
# platform; only the current platform's are built.
function(_rcsw_sources_regex out)
  string(TOLOWER "${RCSW_BUILD_FOR}" _platform)
  list(TRANSFORM ARGN REPLACE "^al$" "al/${_platform}")
  list(JOIN ARGN "|" _alternatives)
  set(${out}
      "src/(${_alternatives})"
      PARENT_SCOPE)
endfunction()

# Monolithic library (optional)
if(RCSW_CONFIG_BUILD_MONOLITHIC)
  _rcsw_sources_regex(_regex ${_RCSW_ALL_PARTS})
  set(_sources ${${PROJECT_NAME}_C_SRC})
  list(
    FILTER
    _sources
    INCLUDE
    REGEX
    "${_regex}")
  libra_add_library(${PROJECT_NAME} ${RCSW_CONFIG_LIBTYPE} ${_sources})
  if(NOT TARGET rcsw::rcsw)
    add_library(rcsw::rcsw ALIAS ${PROJECT_NAME})
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
  _rcsw_sources_regex(_regex ${_RCSW_PARTS_${_component}})
  libra_add_component_library(
    TARGET
    ${PROJECT_NAME}
    COMPONENT
    ${_component}
    SOURCES
    ${${PROJECT_NAME}_C_SRC}
    REGEX
    "${_regex}")
  add_library(rcsw::${_component} ALIAS ${PROJECT_NAME}_${_component})
  list(APPEND _COMPONENT_LIBS ${PROJECT_NAME}_${_component})
endforeach()

# Each library's public headers are those of the parts it contains, so an
# install without some components doesn't contain their headers. Populating
# HEADERS via target_sources() stops LIBRA's automatic calculation, which is
# what we want in this case.
function(_rcsw_add_headers target)
  set(_hdrs)
  foreach(_part IN LISTS ARGN)
    file(GLOB_RECURSE _part_hdrs CONFIGURE_DEPENDS
         "${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/${_part}/*.h")
    list(APPEND _hdrs ${_part_hdrs})
  endforeach()

  target_sources(
    ${target}
    PUBLIC FILE_SET
           HEADERS
           BASE_DIRS
           "${PROJECT_SOURCE_DIR}/include"
           FILES
           ${_hdrs})
endfunction()

foreach(_component IN LISTS _CONFIGURED_COMPONENTS)
  _rcsw_add_headers(${PROJECT_NAME}_${_component} ${_RCSW_PARTS_${_component}})
endforeach()

if(RCSW_CONFIG_BUILD_MONOLITHIC)
  _rcsw_add_headers(${PROJECT_NAME} ${_RCSW_ALL_PARTS})
endif()

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

# The parent target always exists (monolithic or stub) and always compiles
# version.c, so it always needs definitions regardless of
# RCSW_CONFIG_BUILD_MONOLITHIC.
foreach(
  _lib IN
  LISTS _COMPONENT_LIBS
  ITEMS ${PROJECT_NAME})
  _rcsw_apply_compile_defs(${_lib})
endforeach()

# ##############################################################################
# Include directories
# ##############################################################################
# printf's own include directory (${printf_SOURCE_DIR}/src, which contains
# printf/printf.h) normally comes from linking printf. It is also listed here
# because LIBRA's negative compile tests take their -I flags only from this
# project's INCLUDE_DIRECTORIES, not from linked libraries. Once installed,
# printf's header is at <prefix>/include/printf/printf.h, which
# $<INSTALL_INTERFACE:include> already covers.
foreach(
  _lib IN
  LISTS _COMPONENT_LIBS
  ITEMS ${PROJECT_NAME})
  target_include_directories(
    ${_lib}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
           $<BUILD_INTERFACE:${printf_SOURCE_DIR}/src>
           $<INSTALL_INTERFACE:include>)
endforeach()

# ##############################################################################
# Link libraries
# ##############################################################################
# _rcsw_link(COMPONENT LIB...)
#
# Link LIB... to COMPONENT's library, and to the monolithic library (which
# contains every component) if it's built.
function(_rcsw_link component)
  target_link_libraries(${PROJECT_NAME}_${component} PUBLIC ${ARGN})
  if(RCSW_CONFIG_BUILD_MONOLITHIC)
    target_link_libraries(${PROJECT_NAME} PUBLIC ${ARGN})
  endif()
endfunction()

if(_RCSW_POSIX)
  foreach(_component IN LISTS _CONFIGURED_COMPONENTS)
    _rcsw_link(${_component} pthread dl m)
  endforeach()
endif()

# er is part of core.
if("${RCSW_CONFIG_ER_PLUGIN}" STREQUAL "ZLOG")
  _rcsw_link(core zlog)
endif()

# printf backs stdio, wherever stdio is.
if(_RCSW_STDIO_IN_CORE)
  _rcsw_link(core printf::printf)
elseif("stdio" IN_LIST _CONFIGURED_COMPONENTS)
  _rcsw_link(stdio printf::printf)
endif()

# Each component library links the components it needs, so consumers only name
# the components they use.
foreach(_component IN LISTS _CONFIGURED_COMPONENTS)
  foreach(_dep IN LISTS _RCSW_DEPS_${_component})
    target_link_libraries(${PROJECT_NAME}_${_component}
                          PUBLIC ${PROJECT_NAME}_${_dep})
  endforeach()
endforeach()
