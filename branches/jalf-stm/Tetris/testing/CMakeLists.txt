project(TetrisTest)

set(SOLUTIONDIR "${TetrisTest_SOURCE_DIR}/../../")

if(WIN32)
set(LIBEXT "lib")
else()
set(LIBEXT "a")
set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX "-isystem ")
endif()

include_directories(
    ${SOLUTIONDIR}Futile/include
    ${SOLUTIONDIR}Tetris/include)

include_directories(SYSTEM
    ${SOLUTIONDIR}3rdParty/GTest/include
    ${SOLUTIONDIR}3rdParty/Boost
    ${SOLUTIONDIR}3rdParty/STM)

add_executable(TetrisTest
    src/main.cpp
    src/STMTest.cpp
    src/TetrisTest.cpp
    src/TetrisTest.h
    src/NodeCalculatorTest.cpp
    src/NodeCalculatorTest.h
    src/WorkerTest.cpp
    src/WorkerTest.h
    src/WorkerPoolTest.cpp
    src/WorkerPoolTest.h)

set_target_properties(TetrisTest PROPERTIES
    COMPILE_FLAGS "-std=c++0x -Wall -Wextra -Werror ")

target_link_libraries(TetrisTest
    Tetris
    Futile
    ${EXTRA_LIBS}
    ${SOLUTIONDIR}3rdParty/GTest/lib/${PLATFORM}/${ARCH}/libgtest.${LIBEXT}
    ${SOLUTIONDIR}3rdParty/Boost/lib/${PLATFORM}/${ARCH}/libboost_signals.${LIBEXT}
    ${SOLUTIONDIR}3rdParty/Boost/lib/${PLATFORM}/${ARCH}/libboost_thread.${LIBEXT}
    ${SOLUTIONDIR}3rdParty/Poco/lib/${PLATFORM}/${ARCH}/libPocoFoundation.${LIBEXT})
