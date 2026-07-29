cmake_minimum_required(VERSION 3.20)

project(Vivianite)

set(CMAKE_CXX_STANDARD 20)

option(ENABLE_LTO "Enable link-time optimization" OFF)

if(ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT LTO_SUPPORTED)

    if(LTO_SUPPORTED)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-O0 -g -Wall -Wextra)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O3)
    add_compile_options(-flto=auto)
endif()

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

file(GLOB_RECURSE SOURCES
    src/runtime/*.cpp
)

add_library(glad
    glad/src/gl.c
)

target_include_directories(glad PUBLIC
    glad/include
)

add_executable(Vivianite
    ${SOURCES}
)

target_include_directories(Vivianite PRIVATE
    src
)

target_link_libraries(Vivianite PRIVATE
    glad
    glfw
    OpenGL::GL
)
