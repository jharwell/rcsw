# ##############################################################################
# Installation and deployment
# ##############################################################################
# Only applies to POSIX/Linux; embedded targets don't install.
if("${RCSW_BUILD_FOR}" MATCHES "POSIX")
  libra_configure_exports(${PROJECT_NAME})

  # Install every component library
  foreach(_component IN LISTS _CONFIGURED_COMPONENTS)
    libra_install_target(${PROJECT_NAME}_${_component})
  endforeach()

  # Install monolithic library only if built. Its HEADERS file set holds only
  # the configured components' headers; INCLUDE_DIR would install all of them.
  if(RCSW_CONFIG_BUILD_MONOLITHIC)
    libra_install_target(${PROJECT_NAME})
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
  * RCSW_CONFIG_MULTIPROCESS=${RCSW_CONFIG_MULTIPROCESS}\n\
  * RCSW_CONFIG_MULTITHREAD=${RCSW_CONFIG_MULTITHREAD}\n\
  * RCSW_CONFIG_STDIO=${RCSW_CONFIG_STDIO}\n\
  * RCSW_CONFIG_STDIO_GETCHAR=${RCSW_CONFIG_STDIO_GETCHAR}\n\
  * RCSW_CONFIG_STDIO_PUTCHAR=${RCSW_CONFIG_STDIO_PUTCHAR}\n\
  * RCSW_CONFIG_SWBUS=${RCSW_CONFIG_SWBUS}\n\
  * RCSW_CONFIG_TOOL=${RCSW_CONFIG_TOOL}")

  libra_configure_cpack(
    "DEB;RPM"
    ${RCSW_PKG_SUMMARY}
    ${RCSW_PKG_DESCRIPTION}
    "John Harwell"
    "https://jharwell.github.io/rcsw"
    "John Harwell <john.r.harwell@gmail.com>")
endif()
