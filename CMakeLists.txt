cmake_minimum_required(VERSION 3.16)
project(Quanta)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# === Source files ===
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    src/*.cpp
    src/*.hpp
)

# === Filter out main.cpp from SRC_FILES for reuse
list(FILTER SRC_FILES EXCLUDE REGEX ".*/main\\.cpp$")

# === Build main app (includes main.cpp explicitly)
add_executable(quanta ${SRC_FILES} src/main.cpp)
target_include_directories(quanta PRIVATE ${CMAKE_SOURCE_DIR}/src)

# === Test setup ===
enable_testing()

file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS test/test_runner.cpp)

add_executable(quanta_tests ${TEST_SOURCES} ${SRC_FILES})
target_include_directories(quanta_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_test(NAME QuantaTestSuite COMMAND quanta_tests)
