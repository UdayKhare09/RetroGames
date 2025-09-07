# Toolchain file for cross-compiling to Windows x86_64 using mingw-w64
# Usage: cmake -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/mingw_x86_64.cmake ..

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_VERSION 1)

# Adjust these if your mingw-w64 is in a custom prefix
set(MINGW_PREFIX /usr/x86_64-w64-mingw32)

set(CMAKE_C_COMPILER /bin/x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER /bin/x86_64-w64-mingw32-g++)
set(CMAKE_RC_COMPILER /bin/x86_64-w64-mingw32-windres)

# Allow CMake to locate pkg-config (or a cross pkg-config wrapper) from the environment

# Search for libraries under the MinGW prefix first
set(CMAKE_FIND_ROOT_PATH ${MINGW_PREFIX})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Target architecture
set(CMAKE_C_FLAGS "-DWIN32_LEAN_AND_MEAN")
set(CMAKE_CXX_FLAGS "-DWIN32_LEAN_AND_MEAN")

# Adjust RPATH handling
set(CMAKE_SKIP_RPATH TRUE)
