set(
    HELLO_SRC

    "src/main.cc"
)

add_executable(
    hello

    ${HELLO_SRC}
)

target_link_libraries(
    hello

    d0
)