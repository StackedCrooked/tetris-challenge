project(QtTetris)
find_package(Boost)
find_package(Poco)


find_package(Qt5 REQUIRED COMPONENTS Gui Widgets Core)
set(CMAKE_AUTOMOC ON)
qt5_add_resources(QtResources QtTetris.qrc)


add_executable(QtTetris
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
    Poco::Poco)
