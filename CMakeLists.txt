cmake_minimum_required(VERSION 3.10)
project(ATModemServer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(at_server
    src/main.cpp
    src/server.cpp
    src/serial_port.cpp
    src/dictionary.cpp
    src/pattern.cpp
)
target_include_directories(at_server PRIVATE src)
target_link_libraries(at_server util pthread)

enable_testing()
add_executable(tests
    tests/test_main.cpp
    src/pattern.cpp
    src/dictionary.cpp
)
target_include_directories(tests PRIVATE src)
add_test(NAME unit_tests COMMAND tests)
