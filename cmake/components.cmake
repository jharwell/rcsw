# cmake-format: off
# ##############################################################################
# Components
#
# core is always built. It holds everything the rest of rcsw is built on: the
# abstraction layer, data structures, algorithms, event reporting and utils.
# These use each other in cycles, so they're built and linked as one library.
# With the SIMPLE event reporting plugin, er prints through stdio, so stdio is
# part of core too; otherwise stdio is its own component.
#
# The optional components each have an RCSW_CONFIG_<COMPONENT> option. An
# option is forced OFF when the platform can't build the component or a
# component it needs is OFF, so turning off a component also turns off the
# ones that need it.
#
# Sets:
#
#   _RCSW_CORE_PARTS        The directories under src/ and include/rcsw/ that
#                           make up core.
#   _CONFIGURED_COMPONENTS  The components being built, each after the
#                           components it needs.
#   _RCSW_DEPS_<COMPONENT>  The components <COMPONENT> links to.
# ##############################################################################
# cmake-format: on
include(CMakeDependentOption)

if("${RCSW_BUILD_FOR}" MATCHES "POSIX")
  set(_RCSW_POSIX TRUE)
else()
  set(_RCSW_POSIX FALSE)
endif()

if("${RCSW_CONFIG_ER_PLUGIN}" STREQUAL "SIMPLE")
  set(_RCSW_STDIO_IN_CORE TRUE)
else()
  set(_RCSW_STDIO_IN_CORE FALSE)
endif()

set(_RCSW_CORE_PARTS al core version utils)
if(_RCSW_POSIX OR NOT "${LIBRA_STDLIB}" MATCHES "NONE")
  # Without a stdlib ds and er can't be built, and algorithm needs ds.
  list(
    APPEND
    _RCSW_CORE_PARTS
    algorithm
    ds
    er)
endif()
if(_RCSW_STDIO_IN_CORE)
  list(APPEND _RCSW_CORE_PARTS stdio)
endif()

# Optional components, each declared after the ones it needs.
cmake_dependent_option(
  RCSW_CONFIG_STDIO
  "Build the stdio component (always built, as part of core, with the SIMPLE \
event reporting plugin)"
  ON
  "NOT _RCSW_STDIO_IN_CORE"
  ON)
cmake_dependent_option(
  RCSW_CONFIG_MULTIPROCESS
  "Build the multiprocess component"
  ON
  "_RCSW_POSIX"
  OFF)
cmake_dependent_option(
  RCSW_CONFIG_MULTITHREAD
  "Build the multithread component"
  ON
  "_RCSW_POSIX"
  OFF)
cmake_dependent_option(
  RCSW_CONFIG_SWBUS
  "Build the swbus component"
  ON
  "_RCSW_POSIX;RCSW_CONFIG_MULTITHREAD"
  OFF)
cmake_dependent_option(
  RCSW_CONFIG_TOOL
  "Build the tool component"
  ON
  "_RCSW_POSIX"
  OFF)
cmake_dependent_option(
  RCSW_CONFIG_CONSOLE
  "Build the console component"
  ON
  "_RCSW_POSIX"
  OFF)

set(RCSW_OPTIONAL_COMPONENTS
    stdio
    multiprocess
    multithread
    swbus
    console
    tool)

set(_RCSW_DEPS_core "")
set(_RCSW_DEPS_stdio core)
set(_RCSW_DEPS_multiprocess core)
set(_RCSW_DEPS_multithread core)
set(_RCSW_DEPS_swbus core multithread)
set(_RCSW_DEPS_tool core)
set(_RCSW_DEPS_console core)
if(NOT _RCSW_STDIO_IN_CORE)
  set(LIST APPEND RCSW_DEPS_console stdio)
endif()
set(_CONFIGURED_COMPONENTS core)
foreach(_component IN LISTS RCSW_OPTIONAL_COMPONENTS)
  string(TOUPPER "${_component}" _opt)
  if(_component STREQUAL "stdio" AND _RCSW_STDIO_IN_CORE)
    continue()
  endif()
  if(RCSW_CONFIG_${_opt})
    list(APPEND _CONFIGURED_COMPONENTS ${_component})
  endif()
endforeach()
