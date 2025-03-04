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

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

option(ZYDIS_BUILD_TOOLS "" ON)

add_subdirectory("vendor/fmt")
add_subdirectory("vendor/googletest")
add_subdirectory("vendor/zydis")

add_subdirectory("examples/hello")

set(
    D0_SRC

    "include/d0/d0.h"
    "include/d0/defs.h"

    "src/system/memory.cc" "$<$<PLATFORM_ID:Windows>:src/system/win32_memory.cc>" "include/d0/system/memory.h"
    "src/system/library.cc" "$<$<PLATFORM_ID:Windows>:src/system/win32_library.cc>" "include/d0/system/library.h"

    "src/detour/detour.cc" "include/d0/detour/detour.h"
    "src/detour/dynamic_detour.cc" "include/d0/detour/dynamic_detour.h"
    "src/detour/virtual_detour.cc" "include/d0/detour/virtual_detour.h"

    "src/misc/runtime_exception.cc" "include/d0/misc/runtime_exception.h"
    "src/misc/find_pattern.cc" "include/d0/misc/find_pattern.h"

    "src/data/block_allocator.cc" "include/d0/data/block_allocator.h"
    "src/data/bit_map.cc" "include/d0/data/bit_map.h"
    "src/data/near_allocator.cc" "include/d0/data/near_allocator.h"

    "src/misc/logger.cc" "include/d0/misc/logger.h"
    include/d0/misc/result.h
    include/d0/misc/error.h
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
    fmt::fmt
)

target_compile_definitions(
    d0

    PUBLIC "FMT_UNICODE=0"
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

    "src/system/win32_memory_test.cc"
    "src/data/bit_map_test.cc"
    "src/data/block_allocator_test.cc"
    "src/misc/find_pattern_test.cc"
    src/misc/result_test.cc
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