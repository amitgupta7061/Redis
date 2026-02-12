cmake_minimum_required(VERSION 3.10)
project(rediss LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(rediss
    src/main.cpp
    src/store/kv_store.cpp
    src/parser/command_parser.cpp
    src/commands/command_handler.cpp
    src/network/tcp_server.cpp
    src/server/server_app.cpp
)

target_include_directories(rediss PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_compile_options(rediss PRIVATE -Wall -Wextra -Wpedantic)
