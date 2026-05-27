# ==============================================================================
# Unified Cross-Platform C/C++ Development Environment
# ==============================================================================
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# Install native compilers, cross-compilers (mingw-w64), and tools
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    gcc-13 \
    g++-13 \
    clang-18 \
    lldb-18 \
    lld-18 \
    mingw-w64 \
    curl \
    wget \
    git \
    ninja-build \
    python3 \
    python3-pip \
    python3-venv \
    ca-certificates \
    valgrind \
    cppcheck \
    lcov \
    doxygen \
    graphviz \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# Install latest stable CMake and Conan 2.x via pip
RUN pip3 install --break-system-packages cmake conan

WORKDIR /root/workspace

# Establish persistent internal configurations before volume attachments
RUN conan profile detect --force \
    && cp /root/.conan2/profiles/default /root/.conan2/profiles/linux-gcc \
    && CC=clang-18 CXX=clang++-18 conan profile detect --force \
    && mv /root/.conan2/profiles/default /root/.conan2/profiles/linux-clang

RUN printf "[settings]\nos=Windows\narch=x86_64\ncompiler=gcc\ncompiler.version=13\ncompiler.libcxx=libstdc++11\nbuild_type=Release\n\n[buildenv]\nCC=x86_64-w64-mingw32-gcc\nCXX=x86_64-w64-mingw32-g++\nRC=x86_64-w64-mingw32-windres\n" > /root/.conan2/profiles/windows-mingw