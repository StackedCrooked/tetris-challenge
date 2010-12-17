QT += core gui
TARGET = QtTetris
TEMPLATE = app

CONFIG += i386

INCLUDEPATH += \
    ../Tetris/include \
    ../3rdParty/Boost_1_44_0


SOURCES += \
    MainWindow.cpp \
    main.cpp \
    TetrisWidget.cpp

HEADERS += \
    MainWindow.h \
    TetrisWidget.h

OTHER_FILES += \
    CMakeLists.txt

