project(QtTetris)
find_package(boost)
find_package(Poco)


# Using QtCore, QtGui and QtWidgets
find_package(Qt5Core)
find_package(Qt5Gui)
find_package(Qt5Widgets)
add_definitions(${Qt5Core_DEFINITIONS} ${Qt5Gui_DEFINITIONS} ${Qt5Widgets_DEFINITIONS})
include_directories(SYSTEM ${Qt5Core_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS} ${Qt5Widgets_INCLUDE_DIRS})


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
    PRIVATE
    Tetris
    Futile
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Poco::Poco
    boost::boost)
