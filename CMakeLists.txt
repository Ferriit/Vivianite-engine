cmake_minimum_required(VERSION 3.20)

project(Vivianite LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_LTO "Enable link-time optimization" OFF)

if(ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT LTO_SUPPORTED)

    if(LTO_SUPPORTED)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
endif()

# Disable GLFW extras
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

# Build GLFW from submodule/source
add_subdirectory(external/glfw)

file(GLOB_RECURSE SOURCES
    src/runtime/*.cpp
)

# GLAD
add_library(glad
    external/glad/src/gl.c
)

target_include_directories(glad PUBLIC
    external/glad/include
)

# GLM (header-only)
add_library(glm INTERFACE)

target_include_directories(glm INTERFACE
    external/glm
)

# OpenGL
find_package(OpenGL REQUIRED)

# Executable
add_executable(Vivianite
    ${SOURCES}
)

target_include_directories(Vivianite PRIVATE
    src
)

target_link_libraries(Vivianite PRIVATE
    glad
    glm
    glfw
    OpenGL::GL
)

if(MSVC)
    target_compile_options(Vivianite PRIVATE
        /W4
    )
else()
    target_compile_options(Vivianite PRIVATE
        -Wall
        -Wextra
    )

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(Vivianite PRIVATE
            -O0
            -g
        )
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(Vivianite PRIVATE
            -O3
        )
    endif()
endif()
