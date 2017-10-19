cmake_minimum_required(VERSION 3.0)


# Global g++ flags:
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O2 -march=native -Wall -pedantic -Wno-sign-compare -Wno-deprecated-declarations")
set(CMAKE_LD_FLAGS "${CMAKE_LD_FLAGS} -pthread")


add_subdirectory(3rdParty/gtest)
add_subdirectory(Futile)
add_subdirectory(Tetris)
add_subdirectory(Tetris/testing)
add_subdirectory(QtTetris)
