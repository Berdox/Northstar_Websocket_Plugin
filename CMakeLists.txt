cmake_minimum_required(VERSION 3.15)

project(NS_WS_plugin LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(NS_WS_plugin SHARED 
    src/main.c
    src/interfaces/interface.h
    src/interfaces/interface.c
    src/interfaces/ns_interface.h
    src/interfaces/ns_interface.c
    src/interfaces/engine_interface.h
    src/interfaces/engine_interface.c
    src/interfaces/plugin_callbacks.h
    src/interfaces/plugin_callbacks.c
    src/interfaces/sys.h
    src/interfaces/sys.c
    src/interfaces/cvar.h
    src/interfaces/cvar.c
    src/squirrel/squirrel.h
    src/squirrel/squirrel.c
    src/squirrel/sqvm.h
    src/cvar/cvar.h
    src/cvar/cvar.c
    src/bindings/native_closures.h
    src/bindings/native_closures.c
    src/bindings/module.h
    src/bindings/module.c
    src/bindings/relay.h
    src/bindings/relay.c
    src/bindings/assert.h
    )



set_target_properties(NS_WS_plugin PROPERTIES 
    PREFIX ""
    OUTPUT_NAME "Websocket_Plugin"
    LINKER_LANGUAGE C
)

#target_include_directories(NS_WS_plugin PRIVATE "include")

target_include_directories(NS_WS_plugin PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

find_package(Libwebsockets REQUIRED)
target_link_libraries(NS_WS_plugin PRIVATE
    websockets          
    ws2_32
    ssl
    crypto
)