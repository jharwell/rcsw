# Shared helpers for the rcsw integration tests.
#
# These tests check an installed rcsw, the way a downstream project would use
# it. Unless RCSW_PREFIX is set, setup_suite.bash first builds rcsw from this
# checkout and installs it into a temporary prefix, setting everything below;
# see there for choosing what it installs. So these all work:
#
#   bats tests/integration                       # every component, STATIC
#   RCSW_ITEST_TARGET=swbus RCSW_ITEST_LIBTYPE=SHARED bats tests/integration
#   ctest -L integration                         # what clibra runs
#
# To test an existing install instead, set:
#
#   RCSW_PREFIX      Install prefix of the rcsw under test.
#   RCSW_COMPONENTS  Space-separated components the install is expected to
#                    contain, e.g. "core" or "core multithread swbus". Tests for
#                    components not listed are skipped. Required with
#                    RCSW_PREFIX.
#   RCSW_MONOLITHIC  ON if the monolithic rcsw::rcsw library was installed;
#                    consumers then link it instead of rcsw::<component>.
#                    Default OFF.
#   RCSW_ER_PLUGIN   ER plugin rcsw was built with. Default LOG4CL.
#   RCSW_SOURCE_DIR  rcsw source/build checkout; installed files must not
#                    reference it. Optional.
#
# e.g.:
#
#   RCSW_PREFIX=/tmp/rcsw-install RCSW_COMPONENTS="core" \
#     bats --print-output-on-failure tests/integration
#
# Requires the bats-support and bats-assert libraries, found through
# BATS_LIB_PATH (default /usr/lib/bats, where Ubuntu's bats-support and
# bats-assert packages install them).

RCSW_INTEGRATION_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Every optional component rcsw can be configured with. core is always built.
RCSW_OPTIONAL_COMPONENTS="multiprocess multithread stdio swbus tool"

# Every header directory (include/rcsw/<dir>) rcsw can install.
RCSW_HEADER_DIRS="al algorithm core ds er multiprocess multithread stdio swbus tool utils version"

# owner DIR
#
# Print the component whose library contains include/rcsw/DIR, and the code
# that goes with it. This mirrors _RCSW_CORE_PARTS in cmake/components.cmake:
# core contains al, algorithm, ds, er, utils and version, and stdio too with
# the SIMPLE ER plugin.
owner() {
  case "$1" in
    al | algorithm | core | ds | er | utils | version)
      echo core
      ;;
    stdio)
      if [ "${RCSW_ER_PLUGIN:-LOG4CL}" = "SIMPLE" ]; then
        echo core
      else
        echo stdio
      fi
      ;;
    *)
      echo "$1"
      ;;
  esac
}

# parts COMPONENT
#
# Print the header directories COMPONENT's library contains, one per line.
parts() {
  local dir
  for dir in ${RCSW_HEADER_DIRS}; do
    [ "$(owner "${dir}")" = "$1" ] && echo "${dir}"
  done
  return 0
}

# Per-test setup shared by every test file.
common_setup() {
    # Vendored copies in tests/integration/lib first, then any system ones.
    BATS_LIB_PATH="${RCSW_INTEGRATION_DIR}/../lib${BATS_LIB_PATH:+:${BATS_LIB_PATH}}"
    bats_load_library bats-support
    bats_load_library bats-assert
    require_env
}

require_env() {
  : "${RCSW_PREFIX:?RCSW_PREFIX must be set to the rcsw install prefix}"
  : "${RCSW_COMPONENTS:?RCSW_COMPONENTS must list the components under test}"
  if [ ! -d "${RCSW_PREFIX}" ]; then
    echo "RCSW_PREFIX=${RCSW_PREFIX} does not exist" >&2
    return 1
  fi

  # Catch names that aren't components, such as parts of core from before
  # they were merged into it.
  local c bad=0
  for c in ${RCSW_COMPONENTS}; do
    if [ "${c}" = "core" ] || [[ " ${RCSW_OPTIONAL_COMPONENTS} " == *" ${c} "* ]]; then
      if [ "${c}" = "stdio" ] && [ "$(owner stdio)" = "core" ]; then
        echo "RCSW_COMPONENTS: stdio is part of core with the" \
          "${RCSW_ER_PLUGIN} ER plugin; list core instead" >&2
        bad=1
      fi
    elif [[ " ${RCSW_HEADER_DIRS} " == *" ${c} "* ]]; then
      echo "RCSW_COMPONENTS: ${c} is part of core, not a component;" \
        "list core instead" >&2
      bad=1
    else
      echo "RCSW_COMPONENTS: ${c} is not an rcsw component (components:" \
        "core ${RCSW_OPTIONAL_COMPONENTS})" >&2
      bad=1
    fi
  done
  return ${bad}
}

# True if COMPONENT is one of the components under test.
under_test() {
  [[ " ${RCSW_COMPONENTS} " == *" $1 "* ]]
}

# Skip the current test unless COMPONENT is under test.
skip_unless_under_test() {
  under_test "$1" || skip "component $1 is not under test"
}

# Skip the current test unless the component that owns header directory DIR
# is under test.
skip_unless_owner_under_test() {
  local component
  component=$(owner "$1")
  under_test "${component}" ||
    skip "$1 is part of ${component}, which is not under test"
}

# write_consumer DIR COMPONENT KIND SOURCE...
#
# Write a CMake project in DIR that finds rcsw and builds SOURCE... against
# COMPONENT (or against rcsw::rcsw if RCSW_MONOLITHIC=ON). KIND is "objects"
# (compile only) or "executable" (compile and link).
write_consumer() {
  local dir=$1 component=$2 kind=$3
  shift 3

  local find libs
  if [ "${RCSW_MONOLITHIC:-OFF}" = "ON" ]; then
    find="find_package(rcsw REQUIRED)"
    libs="rcsw::rcsw"
  else
    find="find_package(rcsw REQUIRED COMPONENTS ${component})"
    libs="rcsw::${component}"
  fi

  local target
  if [ "${kind}" = "objects" ]; then
    target="add_library(consumer OBJECT $*)"
  else
    target="add_executable(consumer $*)"
  fi

  mkdir -p "${dir}"
  cat > "${dir}/CMakeLists.txt" << CMAKE
cmake_minimum_required(VERSION 3.20)
project(rcsw_consumer LANGUAGES C)
${find}
${target}
target_link_libraries(consumer PRIVATE ${libs})
CMAKE
}

# configure_consumer DIR [CMAKE_ARGS...]
configure_consumer() {
  local dir=$1
  shift
  cmake -S "${dir}" -B "${dir}/build" -DCMAKE_PREFIX_PATH="${RCSW_PREFIX}" "$@"
}

# build_consumer DIR
#
# Configure and build the project in DIR, echoing the generated files first so
# a failure shows exactly what was compiled.
build_consumer() {
  local dir=$1
  echo "--- ${dir}/CMakeLists.txt"
  cat "${dir}/CMakeLists.txt"
  configure_consumer "${dir}" && cmake --build "${dir}/build" --verbose
}

# check_headers DIR
#
# Compile each top-level header in include/rcsw/DIR in its own translation
# unit, against only what find_package() provides for the component that owns
# DIR. A header that compiles only because something else was included first
# fails here.
#
# Subdirectories (er/plugin/, al/posix/, al/baremetal/) are platform- or
# plugin-specific; they're reached through the top-level headers.
check_headers() {
  local hdir=$1
  local component
  component=$(owner "${hdir}")
  local dir="${BATS_TEST_TMPDIR}/headers-${hdir}"
  local headers=("${RCSW_PREFIX}/include/rcsw/${hdir}"/*.h)

  if [ ! -e "${headers[0]}" ]; then
    echo "no headers installed in ${RCSW_PREFIX}/include/rcsw/${hdir}"
    return 1
  fi

  mkdir -p "${dir}"
  local sources=() header name
  for header in "${headers[@]}"; do
    name=$(basename "${header}" .h)
    {
      echo "#include \"rcsw/${hdir}/${name}.h\""
      # Keep the translation unit non-empty for -Wpedantic.
      echo "extern int rcsw_header_check_${name};"
    } > "${dir}/${name}.c"
    sources+=("${name}.c")
  done

  write_consumer "${dir}" "${component}" objects "${sources[@]}"
  build_consumer "${dir}"
}

# check_link NAME
#
# Build consumers/NAME/main.c against the component that owns NAME's code (see
# owner()), link it, and run it.
check_link() {
  local name=$1
  local component
  component=$(owner "${name}")
  local dir="${BATS_TEST_TMPDIR}/link-${name}"

  mkdir -p "${dir}"
  cp "${RCSW_INTEGRATION_DIR}/consumers/${name}/main.c" "${dir}/main.c"
  write_consumer "${dir}" "${component}" executable main.c
  build_consumer "${dir}" && "${dir}/build/consumer"
}
