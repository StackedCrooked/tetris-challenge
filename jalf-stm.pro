#-------------------------------------------------
#
# Project created by QtCreator 2013-12-15T07:07:27
#
#-------------------------------------------------

QT       += core gui
CONFIG += c++11
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = jalf-stm
TEMPLATE = app

INCLUDEPATH += 3rdParty/stm Tetris/include Futile/include /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/include/c++/v1

MAKEFLAGS = -j11
QMAKE_CXXFLAGS += -std=c++11 -isystem /usr/local/include

QMAKE_LFLAGS +=  -L/usr/local/lib -lPocoFoundation -ltbb -lboost_system-mt -lboost_thread-mt

HEADERS += \
    Futile/include/Futile/Array.h \
    Futile/include/Futile/Assert.h  \
    Futile/include/Futile/Atomic.h  \
    Futile/include/Futile/AutoPtrSupport.h  \
    Futile/include/Futile/Boost.h  \
    Futile/include/Futile/Config.h  \
    Futile/include/Futile/GenericGrid.h  \
    Futile/include/Futile/LeakDetector.h  \
    Futile/include/Futile/Logger.h  \
    Futile/include/Futile/Logging.h  \
    Futile/include/Futile/MainThread.h  \
    Futile/include/Futile/MainThreadImpl.h  \
    Futile/include/Futile/MakeString.h  \
    Futile/include/Futile/PrettyPrint.h  \
    Futile/include/Futile/Singleton.h  \
    Futile/include/Futile/STMSupport.h  \
    Futile/include/Futile/Stopwatch.h  \
    Futile/include/Futile/Threading.h  \
    Futile/include/Futile/Timer.h  \
    Futile/include/Futile/TypedWrapper.h  \
    Futile/include/Futile/Types.h  \
    Futile/include/Futile/Worker.h  \
    Futile/include/Futile/WorkerPool.h  \
    QtTetris/MainWindow.h  \
    QtTetris/Model.h  \
    QtTetris/NewGameDialog.h  \
    QtTetris/QtMainThread.h  \
    QtTetris/TetrisWidget.h  \
    Tetris/include/Tetris/AbstractWidget.h  \
    Tetris/include/Tetris/AISupport.h  \
    Tetris/include/Tetris/Block.h  \
    Tetris/include/Tetris/BlockFactory.h  \
    Tetris/include/Tetris/BlockType.h  \
    Tetris/include/Tetris/BlockTypes.h  \
    Tetris/include/Tetris/Computer.h  \
    Tetris/include/Tetris/ComputerPlayer.h  \
    Tetris/include/Tetris/Direction.h  \
    Tetris/include/Tetris/Evaluator.h  \
    Tetris/include/Tetris/Game.h  \
    Tetris/include/Tetris/GameOver.h  \
    Tetris/include/Tetris/GameState.h  \
    Tetris/include/Tetris/GameStateComparator.h  \
    Tetris/include/Tetris/GameStateNode.h  \
    Tetris/include/Tetris/GameStateStats.h  \
    Tetris/include/Tetris/Gravity.h  \
    Tetris/include/Tetris/Grid.h  \
    Tetris/include/Tetris/MultiplayerGame.h  \
    Tetris/include/Tetris/MultiThreadedNodeCalculator.h  \
    Tetris/include/Tetris/NodeCalculator.h  \
    Tetris/include/Tetris/NodeCalculatorImpl.h  \
    Tetris/include/Tetris/NodePtr.h  \
    Tetris/include/Tetris/Player.h  \
    Tetris/include/Tetris/PlayerType.h  \
    Tetris/include/Tetris/SimpleGame.h  \
    Tetris/include/Tetris/SingleThreadedNodeCalculator.h  \
    Tetris/include/Tetris/Unicode.h  \
    Tetris/include/Tetris/Utilities.h

SOURCES += \
    Futile/src/LeakDetector.cpp \
    Futile/src/Logger.cpp \
    Futile/src/Logging.cpp \
    Futile/src/MainThread.cpp \
    Futile/src/PrettyPrint.cpp \
    Futile/src/Stopwatch.cpp \
    Futile/src/Threading.cpp \
    Futile/src/Timer.cpp \
    Futile/src/Worker.cpp \
    Futile/src/WorkerPool.cpp \
    QtTetris/main.cpp \
    QtTetris/MainWindow.cpp \
    QtTetris/Model.cpp \
    QtTetris/NewGameDialog.cpp \
    QtTetris/QtMainThread.cpp \
    QtTetris/TetrisWidget.cpp \
    Tetris/src/AbstractWidget.cpp \
    Tetris/src/AISupport.cpp \
    Tetris/src/Block.cpp \
    Tetris/src/BlockFactory.cpp \
    Tetris/src/BlockType.cpp \
    Tetris/src/Computer.cpp \
    Tetris/src/ComputerPlayer.cpp \
    Tetris/src/Evaluator.cpp \
    Tetris/src/Game.cpp \
    Tetris/src/GameState.cpp \
    Tetris/src/GameStateComparator.cpp \
    Tetris/src/GameStateNode.cpp \
    Tetris/src/Gravity.cpp \
    Tetris/src/MultiplayerGame.cpp \
    Tetris/src/MultiThreadedNodeCalculator.cpp \
    Tetris/src/NodeCalculator.cpp \
    Tetris/src/NodeCalculatorImpl.cpp \
    Tetris/src/Player.cpp \
    Tetris/src/SimpleGame.cpp \
    Tetris/src/SingleThreadedNodeCalculator.cpp \
    Tetris/src/Unicode.cpp
