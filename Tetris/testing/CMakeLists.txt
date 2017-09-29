include_directories(${SOLUTIONDIR}Futile/include
                    ${SOLUTIONDIR}Tetris/include)


set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX "-isystem ")

include_directories(SYSTEM
    ${SOLUTIONDIR}3rdParty/Boost
    ${SOLUTIONDIR}3rdParty/GTest/include
    ${SOLUTIONDIR}3rdParty/stm
    /opt/local/include
    ${SOLUTIONDIR}3rdParty/Poco/Foundation/include)


add_executable(TetrisTest src/quick_stm_test.cpp)

#    src/WorkerPoolTest.cpp
#    src/WorkerTest.cpp
#    src/TetrisTest.cpp
#    src/Benchmark.cpp
#    src/ComputerPlayerTest.cpp
#    src/NodeCalculatorTest.cpp
#    src/main.cpp)


set_target_properties(TetrisTest PROPERTIES COMPILE_FLAGS ${FINAL_COMPILE_FLAGS})



#
# Clang requires explicty dylib.
#
if(${CMAKE_CXX_COMPILER} MATCHES ".*clang.*")
set(CLANG_LIBCPP "/usr/local/lib/libc++.1.dylib")
else()
set(CLANG_LIBCPP "")
endif()


target_link_libraries(TetrisTest Tetris Futile pthread
                                 /opt/local/lib/libboost_thread-mt.a
                                 ${SOLUTIONDIR}3rdParty/Poco/lib/${PLATFORM}/${ARCH}/libPocoFoundation.a
                                 ${SOLUTIONDIR}3rdParty/GTest/lib/${PLATFORM}/${ARCH}/libgtest.a
                                 ${CLANG_LIBCPP})
