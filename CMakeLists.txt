cmake_minimum_required(VERSION 3.0)
set(CMAKE_USER_MAKE_RULES_OVERRIDE_CXX ${CMAKE_CURRENT_SOURCE_DIR}/cxx_flag_overrides.cmake)
project("Tetris")

add_compile_options(-std=c++11 -Wall -Wextra -pedantic-errors)

add_subdirectory(3rdParty/gtest)
add_subdirectory(Tetris)
add_subdirectory(Futile)
add_subdirectory(QtTetris)
