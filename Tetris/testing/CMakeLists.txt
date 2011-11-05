include_directories(
    ${SOLUTIONDIR}Futile/include
    ${SOLUTIONDIR}Tetris/include
    ${SOLUTIONDIR}3rdParty/Boost
    ${SOLUTIONDIR}3rdParty/GTest/include
    /usr/local/include
    /usr/include)

add_executable(TetrisTest
    src/main.cpp
    src/TetrisTest.cpp
    src/TetrisTest.h
    src/NodeCalculatorTest.cpp
    src/NodeCalculatorTest.h
    src/WorkerTest.cpp
    src/WorkerTest.h
    src/WorkerPoolTest.cpp
    src/WorkerPoolTest.h)

target_link_libraries(TetrisTest
    Tetris
    Futile
    PocoFoundation
    ${EXTRA_LIBS}
    "${BOOST_LIB}/libboost_thread.a"
    "${BOOST_LIB}/libboost_signals.a"
    "${SOLUTIONDIR}3rdParty/GTest/lib/${PLATFORM}/${ARCH}/libgtest.a")
