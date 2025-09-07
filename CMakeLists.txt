cmake_minimum_required(VERSION 3.20)
project(retro_games_collection)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_COMPILER "clang++")

set(CMAKE_EXE_LINKER_FLAGS "-fuse-ld=mold")

# Compiler flags for modern C++ and optimization
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)

file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
    "src/*.hpp"
)

add_executable(retro_games_collection ${SOURCES})

target_include_directories(retro_games_collection PRIVATE 
    src
    ${SDL2_TTF_INCLUDE_DIRS}
)

target_link_libraries(retro_games_collection 
    SDL2::SDL2 
    OpenGL::GL 
    OpenGL::GLU
    ${SDL2_TTF_LIBRARIES}
)

target_compile_definitions(retro_games_collection PRIVATE HAS_SDL_TTF)