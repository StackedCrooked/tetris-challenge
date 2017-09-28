project(TetrisTest)

include_directories(
    ${CMAKE_SOURCE_DIR}/Futile/include
    ${CMAKE_SOURCE_DIR}/Tetris/include
)


add_executable(TetrisTest
    src/Driver.cpp
    src/GenericGridTest.cpp
    src/GenericGridTest.h
    src/MemoryPoolTest.cpp
    src/MemoryPoolTest.h
    src/NodeCalculatorTest.cpp
    src/NodeCalculatorTest.h
    src/NodeTest.cpp
    src/NodeTest.h
    src/TetrisTestSuite.cpp
    src/TetrisTestSuite.h
    src/WorkerPoolTest.cpp
    src/WorkerPoolTest.h
    src/WorkerTest.cpp
    src/WorkerTest.h)


target_link_libraries(TetrisTest
    Tetris
    Futile
    PocoFoundation
    ${Boost_LIBRARIES})
    
