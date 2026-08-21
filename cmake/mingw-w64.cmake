# This file is only for Windows/MinGW builds.

set(CMAKE_SYSTEM_NAME Windows)

if(MSYS OR MINGW)
    # MSYS2 / MinGW environment
    set(CMAKE_C_COMPILER gcc)
    set(CMAKE_CXX_COMPILER g++)
    set(CMAKE_RC_COMPILER windres)
else()
    # Linux cross-compilation
    set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
    set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
    set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)
endif()