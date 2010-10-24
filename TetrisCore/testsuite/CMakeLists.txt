include_directories(../include)

add_executable(${EXECUTABLE} 
    src/Driver.cpp 
    src/NodeCalculatorTest.cpp 
    src/TetrisCoreTestSuite.cpp 
    src/WorkerPoolTest.cpp 
    src/WorkerTest.cpp)

set_target_properties(${EXECUTABLE} PROPERTIES COMPILE_FLAGS "-arch i386")
target_link_libraries(${EXECUTABLE} TetrisCore PocoFoundation CppUnit boost_thread)


install(TARGETS         ${EXECUTABLE}
        RUNTIME
        DESTINATION     ${BUILDDIR}${CMAKE_PROJECT_NAME}
        CONFIGURATIONS  ${CONFIGURATION})

