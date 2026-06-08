# Northstar_Websocket_Plugin
A websocket plugin for northstar



# Build

docker build -t cpp-unified-env .

docker run -it --rm `-v "${PWD}:/root/workspace" ` -v conan_cache:/root/.conan2/p ` cpp-unified-env /bin/bash

# 1. Install dependencies using the Windows profile
conan install . --output-folder=build/windows-gcc --profile:build=linux-gcc --profile:host=windows-mingw --build=missing

# 2. Configure and Compile
cmake --preset windows-gcc
cmake --build --preset windows-gcc

# 1. Install dependencies using the Linux profile
conan install . --output-folder=build/linux-gcc --profile:build=linux-gcc --profile:host=linux-gcc --build=missing -s build_type=Debug

# 2. Configure and Compile
cmake --preset linux-gcc
cmake --build --preset linux-gcc