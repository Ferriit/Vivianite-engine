cmake_minimum_required(VERSION 3.20)

project(Vivianite LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 26)
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

# Check GLFW Linux dependencies
if(UNIX AND NOT APPLE)
    set(GLFW_MISSING_DEPENDENCIES "")

    find_package(PkgConfig QUIET)

    if(NOT PKG_CONFIG_FOUND)
        list(APPEND GLFW_MISSING_DEPENDENCIES "pkg-config")
    else()
        pkg_check_modules(X11 x11)
        pkg_check_modules(XRANDR xrandr)
        pkg_check_modules(XINERAMA xinerama)
        pkg_check_modules(XCURSOR xcursor)
        pkg_check_modules(XI xi)
        pkg_check_modules(WAYLAND wayland-client)
        pkg_check_modules(XKBCOMMON xkbcommon)
    endif()

    include(CheckLibraryExists)

    check_library_exists(GL GLX "" HAVE_OPENGL)
    if(NOT X11_FOUND)
        list(APPEND GLFW_MISSING_DEPENDENCIES "libx11-dev")
    endif()

    if(NOT XRANDR_FOUND)
        list(APPEND GLFW_MISSING_DEPENDENCIES "libxrandr-dev")
    endif()

    if(NOT XINERAMA_FOUND)
        list(APPEND GLFW_MISSING_DEPENDENCIES "libxinerama-dev")
    endif()

    if(NOT XCURSOR_FOUND)
        list(APPEND GLFW_MISSING_DEPENDENCIES "libxcursor-dev")
    endif()

    if(NOT XI_FOUND)
        list(APPEND GLFW_MISSING_DEPENDENCIES "libxi-dev")
    endif()

    if(NOT WAYLAND_FOUND)
        list(APPEND GLFW_MISSING_DEPENDENCIES "libwayland-dev")
    endif()

    if(NOT XKBCOMMON_FOUND)
        list(APPEND GLFW_MISSING_DEPENDENCIES "libxkbcommon-dev")
    endif()

    if(GLFW_MISSING_DEPENDENCIES)
        message(STATUS "")
        message(STATUS "Missing GLFW build dependencies:")
        message(STATUS "--------------------------------")

        foreach(DEP ${GLFW_MISSING_DEPENDENCIES})
            message(STATUS "  - ${DEP}")
        endforeach()

        message(STATUS "")
        message(STATUS "Ubuntu/Debian install command:")
        string(REPLACE ";" " " GLFW_INSTALL_COMMAND "${GLFW_MISSING_DEPENDENCIES}")

        message(STATUS "  sudo apt install ${GLFW_INSTALL_COMMAND}")

        message(FATAL_ERROR "Install missing dependencies and rerun CMake.")
    endif()
endif()

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
            -DDBG
        )
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(Vivianite PRIVATE
            -O3
        )
    endif()
endif()
