cmake_minimum_required(VERSION 3.25)

add_subdirectory("vendor/zydis")

project(
    d0

    DESCRIPTION
    "The Despawn Labs C++ library."

    LANGUAGES
    "C" "CXX"
)

option(D0_SHARED "Whether or not to build a shared library." OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)

set(
    D0_SRC

    "include/d0/d0.h"

    "src/platform/win32_memory.cpp" "include/d0/platform/memory.h"

    "src/detour/detour.cc" "include/d0/detour/detour.h"
    "src/detour/dynamic_detour.cc" "include/d0/detour/dynamic_detour.h"
    "src/detour/virtual_detour.cc" "include/d0/detour/virtual_detour.h"
)

add_library(
    d0

    $<$<BOOL:${D0_SHARED}>:SHARED>

    ${D0_SRC}
)

target_include_directories(
    d0

    PUBLIC "include"
    PRIVATE "src"
)

target_link_libraries(
    d0

    zydis
)