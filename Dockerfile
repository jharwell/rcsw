FROM ubuntu:24.04

ARG DEBIAN_FRONTEND=noninteractive

################################################################################
# Bootstrap
################################################################################
RUN set -ex

RUN apt-get update && apt-get upgrade -y
RUN apt-get update && apt-get install dialog apt-utils -y

RUN apt-get update && apt-get install -y software-properties-common && \
    add-apt-repository -y ppa:deadsnakes/ppa && \
    apt-get update

RUN apt-get update && apt-get install sudo

RUN apt-get update && apt-get install -y wget gpg && \
    wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | \
    gpg --dearmor -o /etc/apt/trusted.gpg.d/kitware.gpg && \
    echo 'deb https://apt.kitware.com/ubuntu/ noble main' > /etc/apt/sources.list.d/kitware.list

################################################################################
# Install LIBRA packages
################################################################################
# Core
RUN apt-get update && apt-get install -y \
    git \
    ssh \
    curl \
    make \
    cmake \
    gcc \
    g++ \
    gcc-14 \
    g++-14 \
    git-extras \
    lintian \
    gdb \
    valgrind \
    gcovr


# Devel
RUN apt-get update && apt-get install -y \
    lcov \
    python3-pip \
    file \
    graphviz \
    doxygen \
    curl

RUN pip3 install --break-system-packages \
    sphinx \
    sphinx-rtd-theme \
    sphinx-argparse \
    sphinx-tabs \
    sphinx-last-updated-by-git \
    sphinxcontrib-doxylink \
    autoapi \
    graphviz \
    breathe \
    exhale

################################################################################
# Install RCSW packages
################################################################################
# Dependencies
RUN git clone https://github.com/HardySimpson/zlog.git
RUN cd zlog && \
    mkdir build && cd build && \
    cmake .. && \
    make -j $(nproc) install

# Devel
RUN apt-get update && apt-get install -y catch
