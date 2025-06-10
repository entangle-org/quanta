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