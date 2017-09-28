cmake_minimum_required(VERSION 2.6)
project(QtTetris)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(SOLUTIONDIR "${QtTetris_SOURCE_DIR}/../")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O2 -march=native -Wall -pedantic -Wno-sign-compare")


find_package(Qt5Core)
find_package(Qt5Gui)
find_package(Qt5Widgets)
add_definitions(${Qt5Core_DEFINITIONS})
add_definitions(${Qt5Gui_DEFINITIONS})
add_definitions(${Qt5Widgets_DEFINITIONS})
include_directories(SYSTEM ${Qt5Core_INCLUDE_DIRS})
include_directories(SYSTEM ${Qt5Gui_INCLUDE_DIRS})
include_directories(SYSTEM ${Qt5Widgets_INCLUDE_DIRS})


# Boost: system and thread
find_package(Boost COMPONENTS system thread REQUIRED)
include_directories(SYSTEM ${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})



include_directories(../Futile/include)
include_directories(../Tetris/include)


add_executable(QtTetris
    ${QtTetris_SOURCES}
    main.cpp
    MainWindow.cpp
    Model.cpp
    NewGameDialog.cpp
    TetrisWidget.cpp
    QtMainThread.cpp
    QtTetris.qrc)


target_link_libraries(QtTetris
    Tetris
    Futile
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    PocoFoundation
    Futile
    Tetris
    ${Boost_LIBRARIES})

