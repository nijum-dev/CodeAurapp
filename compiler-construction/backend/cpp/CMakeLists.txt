cmake_minimum_required(VERSION 3.16)

project(CppCompilerBackend LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE CPP_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_executable(cpp_compiler main.cpp ${CPP_SOURCES})

target_include_directories(cpp_compiler PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
