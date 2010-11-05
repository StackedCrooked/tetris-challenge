include_directories(../include)

add_executable(${EXECUTABLE} 
    src/Driver.cpp 
    src/GenericGridTest.cpp
    src/NodeCalculatorTest.cpp 
    src/TetrisCoreTestSuite.cpp 
    src/WorkerPoolTest.cpp 
    src/WorkerTest.cpp)


set_target_properties(${EXECUTABLE} PROPERTIES COMPILE_FLAGS ${FINAL_COMPILE_FLAGS})


set(LIBS TetrisCore PocoFoundation CppUnit boost_thread)
if (UNIX)
set(LIBS ${LIBS} pthread)
endif()

target_link_libraries(${EXECUTABLE} ${LIBS})

