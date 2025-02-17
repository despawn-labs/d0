cmake_minimum_required(VERSION 3.25)


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

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 11)

add_subdirectory("vendor/googletest")
add_subdirectory("vendor/zydis")

add_subdirectory("examples/hello")

set(
    D0_SRC

    "include/d0/defs.h"

    "src/platform/memory.cc" "src/platform/win32_memory.cc" "include/d0/platform/memory.h"

    "src/detour/detour.cc" "include/d0/detour/detour.h"
    "src/detour/dynamic_detour.cc" "include/d0/detour/dynamic_detour.h"
    "src/detour/virtual_detour.cc" "include/d0/detour/virtual_detour.h"
    include/d0/utility/bit_map.h
    src/utility/bit_map.cc
    include/d0/utility/runtime_exception.h
    src/utility/runtime_exception.cc
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

    Zydis
)

target_compile_definitions(
    d0

    PRIVATE "D0_EXPORT"
)

if ("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
    target_compile_definitions(
        d0

        PUBLIC "D0_X64"
    )
else ()
    target_compile_definitions(
        d0

        PUBLIC "D0_X32"
    )
endif ()

set(
    D0_TEST_SRC

    "src/platform/win32_memory_test.cc"
    src/utility/bit_map_test.cc
)

add_executable(
    d0_test

    ${D0_TEST_SRC}
)

target_link_libraries(
    d0_test

    d0
    gtest
    gtest_main
)