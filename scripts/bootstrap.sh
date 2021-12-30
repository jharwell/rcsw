#!/bin/bash
#
# Bootstraps RCSW, including:
#
# - Install .deb dependencies from repositories (ubuntu assumed)
# - Compile and install main project
#
usage() {
    cat << EOF >&2
Usage: $0 [--rroot <dir>] [--cores <n_cores>] [--nosyspkgs ] [--opt] [-h|--help]

--rroot <dir>: The root directory for all repos for the project. All github
               repos will be cloned/built in here. Default=$HOME/research.

--cores: The # cores to use when compiling. Default=$(nproc).

--nosyspkgs: If passed, then do not install system packages (requires sudo
             access). Default=YES (install system packages).

--opt: Perform an optimized build of RCSW. Default=NO.

--prefix: The path to install RCSW and other dependencies
          to. Default=$HOME/.local.

--arch: The target architecture. Can be 'native', 'armhf'. Default=native.

-h|--help: Show this message.
EOF
    exit 1
}

# Make sure script was not run as root or with sudo
if [ $(id -u) = 0 ]; then
    echo "This script cannot be run as root."
    exit 1
fi

repo_root=$HOME/research
install_sys_pkgs="YES"
n_cores=$(nproc)
prefix=$HOME/.local
build_type="DEV"
arch="native"
options=$(getopt -o h --long help,rroot:,cores:,prefix:,arch:,nosyspkgs,opt  -n "BOOTSTRAP" -- "$@")
if [ $? != 0 ]; then usage; exit 1; fi

eval set -- "$options"
while true; do
    case "$1" in
        -h|--help) usage;;
        --rroot) repo_root=$2; shift;;
        --cores) n_cores=$2; shift;;
        --nosyspkgs) install_sys_pkgs="NO";;
        --prefix) prefix=$2; shift;;
        --arch) prefix=$2; shift;;
        --opt) build_type="OPT";;
        --) break;;
        *) break;;
    esac
    shift;
done


################################################################################
# Bootstrap main
################################################################################
function bootstrap_rcsw() {
    # Bootstrap rcsw
    if [ -d rcsw ]; then rm -rf rcsw; fi
    git clone https://github.com/swarm-robotics/rcsw.git
    cd rcsw
    git checkout devel
    git submodule update --init --recursive --remote

    # Build and install rcsw
    if [ "OPT" = "$build_type" ]; then
        er="NONE"
    else
        er="ALL"
    fi
    npm install
    mkdir -p build && cd build
    if [ "native" = "$arch" ]; then
        cmake\
            -DCMAKE_C_COMPILER=gcc-9 \
            -DCMAKE_CXX_COMPILER=g++-9 \
            -DCMAKE_INSTALL_PREFIX=$prefix \
            ..
    elif [ "armhf" = "$arch" ]; then
        cmake\
            -DCMAKE_TOOLCHAIN_FILE=../libra/cmake/arm-linux-gnueabihf-toolchain.cmake \
            -DCMAKE_INSTALL_PREFIX=$prefix \
            ..
    fi
    make -j $n_cores
    cd ..
}

function bootstrap_main() {
    mkdir -p $repo_root && cd $repo_root

    # First, bootstrap LIBRA
    wget \
        --no-cache\
        --no-cookies\
        https://raw.githubusercontent.com/swarm-robotics/libra/devel/scripts/bootstrap.sh \
        -O bootstrap-libra.sh

    chmod +x bootstrap-libra.sh
    libra_syspkgs=$([ "YES" = "$install_sys_pkgs" ] && echo "" || echo "--nosyspkgs")

    # ./bootstrap-libra.sh $libra_syspkgs

    echo -e "********************************************************************************"
    echo -e "RCSW BOOTSTRAP START:"
    echo -e "REPO_ROOT=$repo_root"
    echo -e "N_CORES=$n_cores"
    echo -e "SYSPKGS=$install_sys_pkgs"
    echo -e "BUILD_TYPE=$build_type"
    echo -e "********************************************************************************"
    set -x
    bootstrap_rcsw


    cd ..

    # Made it!
    echo -e "********************************************************************************"
    echo -e "RCSW BOOTSTRAP SUCCESS!"
    echo -e "********************************************************************************"
}

bootstrap_main
