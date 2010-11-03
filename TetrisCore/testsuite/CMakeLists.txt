include_directories(../include)

add_executable(${EXECUTABLE} 
    src/Driver.cpp 
    src/GenericGridTest.cpp
    src/NodeCalculatorTest.cpp 
    src/TetrisCoreTestSuite.cpp 
    src/WorkerPoolTest.cpp 
    src/WorkerTest.cpp)

if(APPLE)
set_target_properties(${EXECUTABLE} PROPERTIES COMPILE_FLAGS "-arch i386")
endif()

target_link_libraries(${EXECUTABLE} TetrisCore PocoFoundation CppUnit boost_thread)


if(APPLE)
install(TARGETS         ${EXECUTABLE}
        RUNTIME
        DESTINATION     ${BUILDDIR}${CMAKE_PROJECT_NAME}
        CONFIGURATIONS  ${CONFIGURATION})
endif()
