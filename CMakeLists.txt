cmake_minimum_required(VERSION 3.31 FATAL_ERROR)

project(rcsw VERSION 1.2.30)

# ##############################################################################
# Packaging bootstrap
# ##############################################################################
# Download CPM.cmake itself
if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)
  file(
    DOWNLOAD
    https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.40.2/CPM.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)
endif()
set(CPM_SOURCE_CACHE
    $ENV{HOME}/.cache/CPM
    CACHE PATH "CPM source cache")

# We want CPM to prefer local packages if they exist. This allows seamless
# mixing of local and remote repos when doing local development.
set(CPM_USE_LOCAL_PACKAGES YES)
include(${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)

# ##############################################################################
# Configuration bootstrap
#
# We use LIBRA, and so need to download it. We do NOT need to install it, since
# CPM makes it available via libra_SOURCE_DIR. We also skip the configure step,
# since we don't need to configure+install it to use it.
# ##############################################################################
cpmaddpackage(
  NAME
  libra
  GIT_REPOSITORY
  https://github.com/jharwell/libra.git
  VERSION
  0.12.8
  GIT_TAG
  v0.12.8-dev.5)

if(libra_ADDED)
  # Downloaded
  list(APPEND CMAKE_MODULE_PATH ${libra_SOURCE_DIR}/cmake)
else()
  # Local package. libra_DIR points to where libra cmake files live (cmake/ in
  # this dir), and we need the parent dir, because everything is included as
  # libra/XX
  list(APPEND CMAKE_MODULE_PATH ${libra_DIR}/..)
endif()

include(libra/version)
libra_extract_version()

project(
  rcsw
  LANGUAGES C CXX
  VERSION ${LIBRA_PROJECT_VERSION_NUMERIC})

message(
  STATUS "Configuring RCSW v${PROJECT_VERSION} (${LIBRA_PROJECT_VERSION})")

include(libra/project)
