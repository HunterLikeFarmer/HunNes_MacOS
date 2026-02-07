cmake_minimum_required(VERSION 3.16)

project(HunNes LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(HunNes
    src/main.cpp
    src/6502.cpp
    src/Controller.cpp
    src/PPU.cpp
    src/RAM.cpp
    src/ROM.cpp
    src/Mapper/CNROM.cpp
    src/Mapper/Mapper.cpp
    src/Mapper/UnROM.cpp
    src/Mapper/MMC1.cpp
    src/Mapper/NROM.cpp
)

target_include_directories(HunNes PRIVATE include)

# Older libstdc++ versions need explicit filesystem linkage.
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
    target_link_libraries(HunNes PRIVATE stdc++fs)
endif()

find_package(SDL2 QUIET CONFIG)
if (NOT TARGET SDL2::SDL2 AND NOT TARGET SDL2::SDL2-static)
    find_package(SDL2 QUIET)
endif()

find_package(PkgConfig QUIET)
if (PkgConfig_FOUND)
    pkg_check_modules(SDL2_PKG QUIET IMPORTED_TARGET sdl2)
endif()

if (TARGET SDL2::SDL2)
    target_link_libraries(HunNes PRIVATE SDL2::SDL2)
elseif(TARGET SDL2::SDL2-static)
    target_link_libraries(HunNes PRIVATE SDL2::SDL2-static)
elseif(SDL2_FOUND)
    target_include_directories(HunNes PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(HunNes PRIVATE ${SDL2_LIBRARIES})
elseif(TARGET PkgConfig::SDL2_PKG)
    target_link_libraries(HunNes PRIVATE PkgConfig::SDL2_PKG)
else()
    message(FATAL_ERROR
        "SDL2 was not found. Install SDL2 development package (e.g. libsdl2-dev) and re-run CMake."
    )
endif()
