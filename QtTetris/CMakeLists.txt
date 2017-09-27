cmake_minimum_required(VERSION 2.6)
project(QtTetris)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
find_package(Qt5Widgets)
find_package(Qt5Core)
find_package(Qt5Gui)

find_package(Boost COMPONENTS system thread REQUIRED)
include_directories(SYSTEM ${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})

get_target_property(QtCore_location Qt5::Core LOCATION)
get_target_property(QtWidgets Qt5::Widgets LOCATION)
get_target_property(QtGui_location Qt5::Gui LOCATION)

include_directories(SYSTEM ${Qt5Widgets_INCLUDE_DIRS})
include_directories(SYSTEM ${Qt5Core_INCLUDE_DIRS})
include_directories(SYSTEM ${Qt5Gui_INCLUDE_DIRS})


add_definitions(${Qt5Widgets_DEFINITIONS})
add_definitions(${Qt5Core_DEFINITIONS})
add_definitions(${Qt5Gui_DEFINITIONS})




set(SOLUTIONDIR "${QtTetris_SOURCE_DIR}/../")
set(BUILD_ROOT ${SOLUTIONDIR}Build/)


set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BUILD_ROOT}lib/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BUILD_ROOT}bin/)



set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra")


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


#
# QtTetris header files
#

set(QtTetris_HEADERS MainWindow.h TetrisWidget.h NewGameDialog.h)


#
# Platform specific options for the creation of the executable
#
if(APPLE)
    set(EXECUTABLE_OPTIONS MACOSX_BUNDLE)
else()
    set(EXECUTABLE_OPTIONS "")
endif()


add_executable(QtTetris ${QtTetris_SOURCES} ${QtTetris_HEADERS_MOC} QtTetris.qrc)


target_link_libraries(QtTetris
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    PocoFoundation
    ${Boost_LIBRARIES}
)

