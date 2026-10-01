# Suite setup for the rcsw integration tests.
#
# With RCSW_PREFIX set, the tests check that existing install; see helpers.bash
# for the other variables to set.
#
# Otherwise, rcsw is built from this checkout and installed into a temporary
# prefix first. That's what CI does, and it's what happens when the tests are
# run with nothing set up, e.g. by ctest or clibra. RCSW_ITEST_TARGET picks what
# to install:
#
#   all (default)  every component, each as its own library
#   core           core only
#   <component>    core, <component>, and the components it needs
#   monolithic     every component, plus the monolithic library
#
# RCSW_ITEST_LIBTYPE is STATIC (default) or SHARED, and RCSW_ER_PLUGIN the ER
# plugin (default LOG4CL). The build is kept in RCSW_ITEST_BUILD_DIR (default
# build/integration in the checkout), so later runs only rebuild what changed.
# Runs that share it, such as ctest running each test file in parallel, take
# turns.

source "$(dirname "${BASH_SOURCE[0]}")/helpers.bash"

setup_suite() {
  if [ -n "${RCSW_PREFIX:-}" ]; then
    return 0
  fi

  RCSW_SOURCE_DIR="$(cd "${RCSW_INTEGRATION_DIR}/../.." && pwd)"
  RCSW_ER_PLUGIN="${RCSW_ER_PLUGIN:-LOG4CL}"
  RCSW_PREFIX="${BATS_SUITE_TMPDIR}/install"
  export RCSW_SOURCE_DIR RCSW_ER_PLUGIN RCSW_PREFIX

  rcsw_itest_select "${RCSW_ITEST_TARGET:-all}" || return 1
  rcsw_itest_install "${RCSW_ITEST_LIBTYPE:-STATIC}"
}

# rcsw_itest_select TARGET
#
# Choose the components to build for TARGET (see above). Exports
# RCSW_COMPONENTS and RCSW_MONOLITHIC for the tests, and sets
# _RCSW_ITEST_FLAGS to the matching RCSW_CONFIG_<COMPONENT> options.
rcsw_itest_select() {
  local target=$1 enable monolithic=OFF

  # A component whose dependencies are off is forced off (see the
  # cmake_dependent_option() calls in cmake/components.cmake), so they're
  # enabled too.
  case "${target}" in
    all) enable=${RCSW_OPTIONAL_COMPONENTS} ;;
    monolithic)
      enable=${RCSW_OPTIONAL_COMPONENTS}
      monolithic=ON
      ;;
    core) enable="" ;;
    swbus) enable="multithread swbus" ;;
    *)
      if [[ " ${RCSW_OPTIONAL_COMPONENTS} " != *" ${target} "* ]]; then
        echo "RCSW_ITEST_TARGET=${target}: expected all, core, monolithic," \
          "or one of: ${RCSW_OPTIONAL_COMPONENTS}" >&2
        return 1
      fi
      enable=${target}
      ;;
  esac

  local c components="core"
  _RCSW_ITEST_FLAGS=()
  for c in ${RCSW_OPTIONAL_COMPONENTS}; do
    if [[ " ${enable} " == *" ${c} "* ]]; then
      _RCSW_ITEST_FLAGS+=("-DRCSW_CONFIG_${c^^}=ON")
      # With the SIMPLE ER plugin, stdio is part of core.
      [ "$(owner "${c}")" = "${c}" ] && components+=" ${c}"
    else
      _RCSW_ITEST_FLAGS+=("-DRCSW_CONFIG_${c^^}=OFF")
    fi
  done

  export RCSW_COMPONENTS=${components}
  export RCSW_MONOLITHIC=${monolithic}
}

# rcsw_itest_install LIBTYPE
#
# Configure, build and install rcsw into RCSW_PREFIX. The output goes to a log,
# whose end is shown if anything fails.
rcsw_itest_install() {
  local libtype=$1
  local build=${RCSW_ITEST_BUILD_DIR:-${RCSW_SOURCE_DIR}/build/integration}
  local log="${BATS_SUITE_TMPDIR}/rcsw-install.log"

  mkdir -p "${build}"
  if ! (
    flock 9
    cd "${RCSW_SOURCE_DIR}" &&
      cmake --preset debug -B "${build}" \
        -DLIBRA_TESTS=OFF \
        -DRCSW_BUILD_FOR=POSIX \
        -DRCSW_CONFIG_LIBTYPE="${libtype}" \
        -DRCSW_CONFIG_BUILD_MONOLITHIC="${RCSW_MONOLITHIC}" \
        -DRCSW_CONFIG_ER_PLUGIN="${RCSW_ER_PLUGIN}" \
        -DRCSW_CONFIG_PTR_ALIGN=4 \
        -DCMAKE_INSTALL_PREFIX="${RCSW_PREFIX}" \
        "${_RCSW_ITEST_FLAGS[@]}" &&
      cmake --build "${build}" --parallel "$(nproc 2>/dev/null || echo 2)" &&
      cmake --install "${build}"
  ) 9> "${build}.lock" > "${log}" 2>&1; then
    echo "Building and installing rcsw failed; end of ${log}:" >&2
    tail -n 60 "${log}" >&2
    return 1
  fi
}
