include_directories(${SOLUTIONDIR}Futile/include
                    ${SOLUTIONDIR}Tetris/include)


set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX "-isystem ")

include_directories(SYSTEM
    ${SOLUTIONDIR}3rdParty/Boost
    ${SOLUTIONDIR}3rdParty/GTest/include
    ${SOLUTIONDIR}3rdParty/stm
    ${SOLUTIONDIR}3rdParty/Poco/Foundation/include)


add_executable(TetrisTest
    src/STMTest.cpp
    src/WorkerPoolTest.cpp
    src/WorkerTest.cpp
    src/TetrisTest.cpp
    src/Benchmark.cpp
    src/ComputerPlayerTest.cpp
    src/NodeCalculatorTest.cpp
    src/main.cpp)


set(CXX_FLAGS "-std=c++0x -Wall -Wextra -Werror")
if(${CMAKE_CXX_COMPILER} MATCHES ".*clang.*")
    set(CXX_FLAGS "${CXX_FLAGS} -stdlib=libc++ -Qunused-arguments ")
endif()
set_target_properties(TetrisTest  PROPERTIES COMPILE_FLAGS ${CXX_FLAGS})

target_link_libraries(TetrisTest Tetris Futile pthread
                                 ${SOLUTIONDIR}3rdParty/Boost/lib/${PLATFORM}/${ARCH}/libboost_thread.a
                                 ${SOLUTIONDIR}3rdParty/Poco/lib/${PLATFORM}/${ARCH}/libPocoFoundation.a
                                 ${SOLUTIONDIR}3rdParty/GTest/lib/${PLATFORM}/${ARCH}/libgtest.a)
