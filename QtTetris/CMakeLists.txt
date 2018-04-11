project(QtTetris)


#
# Use QtCore
#
find_package(Qt5Core)
add_definitions(${Qt5Core_DEFINITIONS})
include_directories(SYSTEM ${Qt5Core_INCLUDE_DIRS})


#
# Use QtGui
#
find_package(Qt5Gui)
add_definitions(${Qt5Gui_DEFINITIONS})
include_directories(SYSTEM ${Qt5Gui_INCLUDE_DIRS})


#
# Use QtWidgets
#
find_package(Qt5Widgets)
add_definitions(${Qt5Widgets_DEFINITIONS})
include_directories(SYSTEM ${Qt5Widgets_INCLUDE_DIRS})


#
# Use Boost System and Thread libraries
#
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS system thread REQUIRED)
include_directories(SYSTEM ${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})


#
# Use Poco Foundation
#
set(Poco_USE_STATIC_LIBS ON)
find_package(Poco COMPONENTS Foundation REQUIRED)
include_directories(SYSTEM ${Poco_INCLUDE_DIR})
link_directories(${Poco_LIBRARY_DIR})


#
# Depend on Futile and Tetris
#
include_directories(
    ${CMAKE_SOURCE_DIR}/Futile/include
    ${CMAKE_SOURCE_DIR}/Tetris/include)


qt5_wrap_cpp(MocFiles MainWindow.h TetrisWidget.h NewGameDialog.h)
qt5_add_resources(QtResources QtTetris.qrc)


add_executable(QtTetris
    ${MocFiles}
    ${QtResources}
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
    ${Boost_LIBRARIES})
