QT += core gui
TARGET = QtTetris
TEMPLATE = app
CONFIG += i386


INCLUDEPATH += \
    Tetris/include \
    3rdParty/Boost_1_44_0 \
    3rdParty/Poco/Foundation/include


SOURCES += \
    QtTetris/MainWindow.cpp \
    QtTetris/main.cpp \
    TetrisWidget.cpp \
    Tetris/src/AbstractWidget.cpp \
    Tetris/src/AISupport.cpp \
    Tetris/src/Allocator.cpp \
    Tetris/src/Assert.cpp \
    Tetris/src/Block.cpp \
    Tetris/src/BlockFactory.cpp \
    Tetris/src/BlockMover.cpp \
    Tetris/src/BlockType.cpp \
    Tetris/src/ComputerPlayer.cpp \
    Tetris/src/Evaluator.cpp \
    Tetris/src/Game.cpp \
    Tetris/src/GameState.cpp \
    Tetris/src/GameStateComparator.cpp \
    Tetris/src/GameStateNode.cpp \
    Tetris/src/Gravity.cpp \
    Tetris/src/Logger.cpp \
    Tetris/src/Logging.cpp \
    Tetris/src/MainThread.cpp \
    Tetris/src/MultiplayerGame.cpp \
    Tetris/src/MultiThreadedNodeCalculator.cpp \
    Tetris/src/NodeCalculator.cpp \
    Tetris/src/NodeCalculatorImpl.cpp \
    Tetris/src/Player.cpp \
    Tetris/src/PlayerImpl.cpp \
    Tetris/src/SimpleGame.cpp \
    Tetris/src/SingleThreadedNodeCalculator.cpp \
    Tetris/src/Threading.cpp \
    Tetris/src/Unicode.cpp \
    Tetris/src/Worker.cpp \
    Tetris/src/WorkerPool.cpp \
    QtTetris/TetrisWidget.cpp \
    QtTetris/NewGameDialog.cpp


HEADERS += \
    QtTetris/MainWindow.h \
    QtTetris/TetrisWidget.h \
    Tetris/include/Tetris/AbstractWidget.h \
    Tetris/include/Tetris/AISupport.h \
    Tetris/include/Tetris/Allocator.h \
    Tetris/include/Tetris/Array.h \
    Tetris/include/Tetris/Assert.h \
    Tetris/include/Tetris/AutoPtrSupport.h \
    Tetris/include/Tetris/Block.h \
    Tetris/include/Tetris/BlockFactory.h \
    Tetris/include/Tetris/BlockMover.h \
    Tetris/include/Tetris/BlockType.h \
    Tetris/include/Tetris/BlockTypes.h \
    Tetris/include/Tetris/Boost.h \
    Tetris/include/Tetris/ComputerPlayer.h \
    Tetris/include/Tetris/Config.h \
    Tetris/include/Tetris/Direction.h \
    Tetris/include/Tetris/Evaluator.h \
    Tetris/include/Tetris/ForwardDeclarations.h \
    Tetris/include/Tetris/Game.h \
    Tetris/include/Tetris/GameOver.h \
    Tetris/include/Tetris/GameState.h \
    Tetris/include/Tetris/GameStateComparator.h \
    Tetris/include/Tetris/GameStateNode.h \
    Tetris/include/Tetris/GameStateStats.h \
    Tetris/include/Tetris/GenericGrid.h \
    Tetris/include/Tetris/Gravity.h \
    Tetris/include/Tetris/Grid.h \
    Tetris/include/Tetris/Logger.h \
    Tetris/include/Tetris/Logging.h \
    Tetris/include/Tetris/MainThread.h \
    Tetris/include/Tetris/MainThreadImpl.h \
    Tetris/include/Tetris/MakeString.h \
    Tetris/include/Tetris/MultiplayerGame.h \
    Tetris/include/Tetris/MultithreadedNodeCalculator.h \
    Tetris/include/Tetris/NodeCalculator.h \
    Tetris/include/Tetris/NodeCalculatorImpl.h \
    Tetris/include/Tetris/NodePtr.h \
    Tetris/include/Tetris/Player.h \
    Tetris/include/Tetris/PlayerImpl.h \
    Tetris/include/Tetris/PlayerType.h \
    Tetris/include/Tetris/SimpleGame.h \
    Tetris/include/Tetris/SingleThreadedNodeCalculator.h \
    Tetris/include/Tetris/Tetris.h \
    Tetris/include/Tetris/Threading.h \
    Tetris/include/Tetris/TypedWrapper.h \
    Tetris/include/Tetris/Unicode.h \
    Tetris/include/Tetris/Utilities.h \
    Tetris/include/Tetris/Worker.h \
    Tetris/include/Tetris/WorkerPool.h \
    QtTetris/NewGameDialog.h \
    Tetris/include/Tetris/ThreadingConfiguration.h \
    Tetris/include/Tetris/ThreadingConfigurationBoost.h


OTHER_FILES += \
    QtTetris/CMakeLists.txt \
    Tetris/CMakeLists.txt \
    Tetris/Todo.txt
