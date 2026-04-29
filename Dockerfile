FROM ubuntu:24.04

ARG DEBIAN_FRONTEND=noninteractive

################################################################################
# Bootstrap
################################################################################
RUN set -ex

RUN apt-get update && apt-get -y install \
    dialog \
    apt-utils \
    software-properties-common \
    curl \
    ca-certificates \
    build-essential

RUN apt-get update && apt-get install -y wget gpg && \
    wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | \
    gpg --dearmor -o /etc/apt/trusted.gpg.d/kitware.gpg && \
    echo 'deb https://apt.kitware.com/ubuntu/ noble main' > /etc/apt/sources.list.d/kitware.list

# Install rust
ENV RUSTUP_HOME=/usr/local/rustup \
    CARGO_HOME=/usr/local/cargo \
    PATH=/usr/local/cargo/bin:$PATH
RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y --no-modify-path --profile minimal
RUN rustc --version && cargo --version

# Install uv
COPY --from=ghcr.io/astral-sh/uv:latest /uv /uvx /bin/
RUN uv --version

################################################################################
# Install LIBRA packages
################################################################################
# Core
RUN apt-get update && apt-get install -y \
    git \
    ssh \
    make \
    cmake \
    gcc \
    g++ \
    gcc-14 \
    g++-14 \
    clang-20 \
    clang-tidy-20 \
    clang-format-20 \
    cppcheck \
    cmake-format \
    lintian \
    valgrind \
    gcovr \
    ninja-build


# Devel
RUN apt-get update && apt-get install -y \
    lcov \
    python3-pip \
    file \
    graphviz \
    doxygen \
    curl

################################################################################
# Install RCSW packages
################################################################################
# Dependencies
RUN git clone https://github.com/HardySimpson/zlog.git
RUN cd zlog && \
    mkdir build && cd build && \
    cmake .. && \
    make -j $(nproc) install
