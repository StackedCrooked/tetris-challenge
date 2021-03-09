cmake_minimum_required(VERSION 3.0)

# Override CMake's compiler flags used for Debug and Release builds
set(CMAKE_USER_MAKE_RULES_OVERRIDE_CXX ${CMAKE_CURRENT_SOURCE_DIR}/cxx_flag_overrides.cmake)

project("Tetris")

add_compile_options(-std=c++11 -Wall -Wextra -pedantic)

add_subdirectory(Tetris)
add_subdirectory(Futile)
add_subdirectory(QtTetris)
