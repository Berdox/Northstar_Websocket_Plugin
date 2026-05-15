cmake_minimum_required(VERSION 3.15)

project(NS_WS_plugin LANGUAGES CXX)

if(NOT CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" 
        CACHE FILEPATH "vcpkg toolchain")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(NS_WS_plugin SHARED 
    "src/WebsocketPlugin.cpp"
    "src/WebsocketPlugin.h"
    "src/interfaces/Northstar.cpp"
    "src/interfaces/Northstar.h"
    "src/PluginConfiguration.h"
    "src/interfaces/Northstar/INSSys.h"
    "src/interfaces/Interfaces.h" 
    "src/interfaces/Interfaces.cpp" 
    "src/interfaces/exposed/IPluginCallbacks/IPluginCallbacks.cpp" 
    "src/interfaces/exposed/IPluginCallbacks/IPluginCallbacks.h"
    "src/interfaces/exposed/IPluginId/IPluginId.cpp"
    "src/interfaces/exposed/IPluginId/IPluginId.h")

set_target_properties(NS_WS_plugin PROPERTIES 
    PREFIX ""
)

set_target_properties(NS_WS_plugin PROPERTIES 
    OUTPUT_NAME "Northstar_Websocket_Plugin"
)

#target_include_directories(NS_WS_plugin PRIVATE "include")

find_package(ixwebsocket CONFIG REQUIRED)
target_link_libraries(NS_WS_plugin PRIVATE ixwebsocket::ixwebsocket bcrypt)

target_compile_features(NS_WS_plugin PRIVATE cxx_std_20)