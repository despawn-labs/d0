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

    "src/sys/memory.cc" "src/sys/win32_memory.cc" "include/d0/sys/memory.h"

    "src/hook/detour.cc" "include/d0/hook/detour.h"
    "src/hook/dynamic_detour.cc" "include/d0/hook/dynamic_detour.h"
    "src/hook/virtual_detour.cc" "include/d0/hook/virtual_detour.h"

    "src/std/runtime_exception.cc" "include/d0/std/runtime_exception.h"

    "src/std/block_allocator.cc" "include/d0/std/block_allocator.h"

    "src/std/bit_map.cc" "include/d0/std/bit_map.h"
    include/d0/std/near_allocator.h
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

    "src/sys/win32_memory_test.cc"

    "src/std/bit_map_test.cc"
    "src/std/block_allocator_test.cc"
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