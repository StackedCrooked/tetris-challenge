cmake_minimum_required(VERSION 3.0)
set(CONAN_SYSTEM_INCLUDES ON)


add_definitions("-std=c++11 -Wall -pedantic-errors -O2 -march=native ")

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()


add_subdirectory(3rdParty/gtest)
add_subdirectory(Tetris)
add_subdirectory(Futile)
add_subdirectory(Tetris/testing)
add_subdirectory(QtTetris)
