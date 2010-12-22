include_directories(../include)


set (TESTSUITE_SOURCES
    src/GenericGridTest.h
    src/NodeCalculatorTest.h
    src/TetrisTestSuite.h
    src/WorkerPoolTest.h
    src/WorkerTest.h
    src/Driver.cpp
    src/GenericGridTest.cpp
    src/NodeCalculatorTest.cpp
    src/TetrisTestSuite.cpp
    src/WorkerPoolTest.cpp
    src/WorkerTest.cpp)


add_executable(${EXECUTABLE} ${TESTSUITE_SOURCES} ${POCO_FOUNDATION_HEADERS})
set_target_properties(${EXECUTABLE} PROPERTIES COMPILE_FLAGS ${FINAL_COMPILE_FLAGS})

set(LIBS Tetris PocoFoundation CppUnit boost_thread)
if (UNIX)
set(LIBS ${LIBS} pthread)
endif()

target_link_libraries(${EXECUTABLE} ${LIBS})

