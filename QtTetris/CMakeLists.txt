cmake_minimum_required(VERSION 2.6)
project(QtTetris)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(SOLUTIONDIR "${QtTetris_SOURCE_DIR}/../")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O2 -march=native -Wall -pedantic -Wno-sign-compare")


find_package(Qt5Widgets)
include_directories(SYSTEM ${Qt5Widgets_INCLUDE_DIRS})
add_definitions(${Qt5Widgets_DEFINITIONS})


find_package(Qt5Core)
include_directories(SYSTEM ${Qt5Core_INCLUDE_DIRS})
add_definitions(${Qt5Core_DEFINITIONS})


find_package(Qt5Gui)
include_directories(SYSTEM ${Qt5Gui_INCLUDE_DIRS})
add_definitions(${Qt5Gui_DEFINITIONS})


find_package(Boost COMPONENTS system thread REQUIRED)
include_directories(SYSTEM ${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})





include_directories(${SOLUTIONDIR}/Futile/include)
include_directories(${SOLUTIONDIR}/Tetris/include)


set(QtTetris_SOURCES
    ${SOLUTIONDIR}/Tetris/src/AbstractWidget.cpp
    ${SOLUTIONDIR}/Tetris/src/AISupport.cpp
    ${SOLUTIONDIR}/Tetris/src/Block.cpp
    ${SOLUTIONDIR}/Tetris/src/BlockFactory.cpp
    ${SOLUTIONDIR}/Tetris/src/BlockMover.cpp
    ${SOLUTIONDIR}/Tetris/src/BlockType.cpp
    ${SOLUTIONDIR}/Tetris/src/ComputerPlayer.cpp
    ${SOLUTIONDIR}/Tetris/src/Evaluator.cpp
    ${SOLUTIONDIR}/Tetris/src/Game.cpp
    ${SOLUTIONDIR}/Tetris/src/GameImpl.cpp
    ${SOLUTIONDIR}/Tetris/src/GameState.cpp
    ${SOLUTIONDIR}/Tetris/src/GameStateComparator.cpp
    ${SOLUTIONDIR}/Tetris/src/GameStateNode.cpp
    ${SOLUTIONDIR}/Tetris/src/Gravity.cpp
    ${SOLUTIONDIR}/Tetris/src/MultiplayerGame.cpp
    ${SOLUTIONDIR}/Tetris/src/MultiThreadedNodeCalculator.cpp
    ${SOLUTIONDIR}/Tetris/src/NodeCalculator.cpp
    ${SOLUTIONDIR}/Tetris/src/NodeCalculatorImpl.cpp
    ${SOLUTIONDIR}/Tetris/src/Player.cpp
    ${SOLUTIONDIR}/Tetris/src/SingleThreadedNodeCalculator.cpp
    ${SOLUTIONDIR}/Tetris/src/Unicode.cpp
    ${SOLUTIONDIR}/Futile/src/LeakDetector.cpp
    ${SOLUTIONDIR}/Futile/src/Logger.cpp
    ${SOLUTIONDIR}/Futile/src/Logging.cpp
    ${SOLUTIONDIR}/Futile/src/MainThread.cpp
    ${SOLUTIONDIR}/Futile/src/Threading.cpp
    ${SOLUTIONDIR}/Futile/src/Worker.cpp
    ${SOLUTIONDIR}/Futile/src/WorkerPool.cpp
    main.cpp
    MainWindow.cpp
    Model.cpp
    NewGameDialog.cpp
    TetrisWidget.cpp
    QtMainThread.cpp)


add_executable(
    QtTetris
    ${QtTetris_SOURCES}
    QtTetris.qrc
)


target_link_libraries(QtTetris
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    PocoFoundation
    ${Boost_LIBRARIES}
)

