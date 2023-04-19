cmake_minimum_required(VERSION 3.1.0)
project(chess_engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/modules)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

file(GLOB CHESS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
list(REMOVE_ITEM CHESS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

# Set up tests in a "test_exe" executable
add_library(Catch INTERFACE)
target_include_directories(Catch INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/catch)

file(GLOB TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp)

add_executable(test_exe ${TEST_SOURCES} ${CHESS_SOURCES})
target_include_directories(test_exe PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(test_exe PRIVATE Catch Threads::Threads)

add_executable(engine src/main.cpp ${CHESS_SOURCES})
target_link_libraries(engine PRIVATE Threads::Threads)

option(Coverage "Run with code coverage" OFF)

# https://stackoverflow.com/questions/37957583/how-to-use-gconv-with-cmake
if (Coverage)
  include(CodeCoverage)
  append_coverage_compiler_flags()
  set(COVERAGE_EXCLUDES "/usr*" "9/")
  setup_target_for_coverage_lcov(
    NAME chess_engine_coverage
    EXECUTABLE test_exe
    DEPENDENCIES test_exe
    )
endif()
