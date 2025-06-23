cmake_minimum_required(VERSION 3.16)
project(Quanta)

set(CMAKE_CXX_STANDARD 20)

# Source files
file(GLOB_RECURSE SOURCES
    src/*.cpp
    src/*.hpp
)

# Add executable
add_executable(quanta ${SOURCES})

# Include headers
target_include_directories(quanta PRIVATE ${CMAKE_SOURCE_DIR}/src)

# Test executable
file(GLOB_RECURSE TEST_SOURCES tests/test_runner.cpp)
add_executable(quanta_tests ${TEST_SOURCES} ${SOURCES})
target_include_directories(quanta_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)

enable_testing()
add_test(NAME QuantaTestSuite COMMAND quanta_tests)