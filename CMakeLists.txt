
# Override the RELEASE build type
set(CMAKE_USER_MAKE_RULES_OVERRIDE_CXX ${CMAKE_CURRENT_SOURCE_DIR}/cxx_flag_overrides.cmake)


project("Tetris")
# NOTE: using "Modern CMake" practices
#  - Youtube: https://www.youtube.com/watch?v=ztrnb-bVVPo&t=30m42s
#  - Slides: Using Modern CMake Patterns to Enforce a Good Modular Design - Mathieu Ropert - CppCon 2017.pdf (https://github.com/CppCon/CppCon2017/blob/master/Tutorials/Using%20Modern%20CMake%20Patterns%20to%20Enforce%20a%20Good%20Modular%20Design/Using%20Modern%20CMake%20Patterns%20to%20Enforce%20a%20Good%20Modular%20Design%20-%20Mathieu%20Ropert%20-%20CppCon%202017.pdf)
#  - Relevant pages: slide 38 to 43
#
# TODO: how do use Conan dependencies in a modern way?
cmake_minimum_required(VERSION 3.0)

# CMake modules should be searched in the build directory because that's where conan will put them.
set(CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})

# Enable ccache.
set(CMAKE_CXX_COMPILER_LAUNCHER "ccache")

# Conan libraries will use "-isystem" so they don't generate compiler warnings.
# This way we don't get compiler warnings from third party code.
set(CONAN_SYSTEM_INCLUDES ON)


add_compile_options(-std=c++11 -Wall -Wno-sign-compare -pedantic-errors)

add_subdirectory(3rdParty/gtest)
add_subdirectory(Tetris)
add_subdirectory(Futile)
add_subdirectory(QtTetris)
