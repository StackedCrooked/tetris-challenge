include_directories(../include)
link_directories(${TETRISTESTSUITE_BINARY_DIR}/TetrisCore)
link_directories(/usr/local/lib)
add_executable(test 
    src/Driver.cpp 
    src/NodeCalculatorTest.cpp 
    src/TetrisCoreTestSuite.cpp 
    src/WorkerPoolTest.cpp 
    src/WorkerTest.cpp)
target_link_libraries(test Tetris PocoFoundation CppUnit boost_thread)
