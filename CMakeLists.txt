cmake_minimum_required(VERSION 3.16)

project(nexus LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(nexus
    src/main.cpp
    src/lexer.cpp
    src/parser.cpp
    src/runtime.cpp
    src/value.cpp
)

target_include_directories(nexus PRIVATE include)

if (MSVC)
    target_compile_options(nexus PRIVATE /W4)
else()
    target_compile_options(nexus PRIVATE -Wall -Wextra -Wpedantic)
endif()
