cmake_minimum_required(VERSION 3.20)

project(Vivianite LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_LTO "Enable link-time optimization" OFF)

if(ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT LTO_SUPPORTED)

    if(LTO_SUPPORTED)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
endif()

# ------------------------------------------------------------
# GLFW
# ------------------------------------------------------------

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

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

        string(
            REPLACE ";"
            " "
            GLFW_INSTALL_COMMAND
            "${GLFW_MISSING_DEPENDENCIES}"
        )

        message(
            STATUS
            "  sudo apt install ${GLFW_INSTALL_COMMAND}"
        )

        message(
            FATAL_ERROR
            "Install missing dependencies and rerun CMake."
        )
    endif()
endif()

add_subdirectory(external/glfw)

# ------------------------------------------------------------
# GLAD
# ------------------------------------------------------------

add_library(glad STATIC
    external/glad/src/gl.c
)

target_include_directories(glad PUBLIC
    external/glad/include
)

# ------------------------------------------------------------
# GLM
# ------------------------------------------------------------

add_library(glm INTERFACE)

target_include_directories(glm INTERFACE
    external/glm
)

# ------------------------------------------------------------
# OpenGL
# ------------------------------------------------------------

find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED IMPORTED_TARGET gtkmm-4.0)

# ------------------------------------------------------------
# OpenAL
# ------------------------------------------------------------

if(WIN32)

    add_library(OpenAL::OpenAL SHARED IMPORTED)

    set_target_properties(OpenAL::OpenAL PROPERTIES
        IMPORTED_IMPLIB
            "${CMAKE_SOURCE_DIR}/external/openal/libOpenAL32.a"

        IMPORTED_LOCATION
            "${CMAKE_SOURCE_DIR}/external/openal/OpenAL32.dll"

        INTERFACE_INCLUDE_DIRECTORIES
            "${CMAKE_SOURCE_DIR}/external/openal/include"
    )

else()

    find_package(OpenAL REQUIRED)

endif()

# ------------------------------------------------------------
# Shared engine
# ------------------------------------------------------------

file(GLOB_RECURSE ENGINE_SOURCES
    src/runtime/*.cpp
)

list(REMOVE_ITEM ENGINE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/runtime/main.cpp"
)

add_library(VivianiteEngine STATIC
    ${ENGINE_SOURCES}
)

target_include_directories(VivianiteEngine PUBLIC
    src
)

target_link_libraries(VivianiteEngine PUBLIC
    glad
    glm
    glfw
    OpenGL::GL
    OpenAL::OpenAL
)

# ------------------------------------------------------------
# Runtime
# ------------------------------------------------------------

add_executable(VivianiteRuntime
    src/runtime/engine.cpp
)

target_link_libraries(VivianiteRuntime PRIVATE
    VivianiteEngine
)

# ------------------------------------------------------------
# Editor
# ------------------------------------------------------------

add_executable(VivianiteEditor
    src/editor/main.cpp
)

target_link_libraries(VivianiteEditor PRIVATE
    VivianiteEngine
    PkgConfig::GTKMM
)

# ------------------------------------------------------------
# Compiler options
# ------------------------------------------------------------

if(MSVC)

    target_compile_options(VivianiteEngine PRIVATE /W4)
    target_compile_options(VivianiteRuntime PRIVATE /W4)
    target_compile_options(VivianiteEditor PRIVATE /W4)

else()

    target_compile_options(VivianiteEngine PRIVATE
        -Wall
        -Wextra
    )

    target_compile_options(VivianiteRuntime PRIVATE
        -Wall
        -Wextra
    )

    target_compile_options(VivianiteEditor PRIVATE
        -Wall
        -Wextra
    )

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")

        target_compile_options(VivianiteEngine PRIVATE
            -O0
            -g
            -DDBG
        )

        target_compile_options(VivianiteRuntime PRIVATE
            -O0
            -g
            -DDBG
        )

        target_compile_options(VivianiteEditor PRIVATE
            -O0
            -g
            -DDBG
        )

    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")

        target_compile_options(VivianiteEngine PRIVATE
            -O3
        )

        target_compile_options(VivianiteRuntime PRIVATE
            -O3
        )

        target_compile_options(VivianiteEditor PRIVATE
            -O3
        )

    endif()

endif()

# ------------------------------------------------------------
# Windows DLL
# ------------------------------------------------------------

if(WIN32)

    add_custom_command(TARGET VivianiteRuntime POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_SOURCE_DIR}/external/openal/OpenAL32.dll"
        "$<TARGET_FILE_DIR:VivianiteRuntime>"
    )

endif()
