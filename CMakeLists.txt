cmake_minimum_required(VERSION 3.0.0)
project(chess_engine)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB CHESS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
list(REMOVE_ITEM CHESS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

# Set up tests in a "test_exe" executable
add_library(Catch INTERFACE)
target_include_directories(Catch INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/catch)

file(GLOB TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp)

add_executable(test_exe ${TEST_SOURCES} ${CHESS_SOURCES})
target_include_directories(test_exe PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(test_exe PRIVATE Catch)
