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
    TetrisWidget.cpp \
    ../Tetris/src/Game.cpp \
    ../Tetris/src/AISupport.cpp \
    ../Tetris/src/Allocator.cpp \
    ../Tetris/src/Block.cpp \
    ../Tetris/src/BlockFactory.cpp \
    ../Tetris/src/BlockMover.cpp \
    ../Tetris/src/BlockType.cpp \
    ../Tetris/src/ComputerPlayer.cpp \
    ../Tetris/src/Evaluator.cpp \
    ../Tetris/src/GameState.cpp \
    ../Tetris/src/GameStateComparator.cpp \
    ../Tetris/src/GameStateNode.cpp \
    ../Tetris/src/GenericGrid.cpp \
    ../Tetris/src/Gravity.cpp \
    ../Tetris/src/Logger.cpp \
    ../Tetris/src/Logging.cpp \
    ../Tetris/src/MultiThreadedNodeCalculator.cpp \
    ../Tetris/src/NodeCalculator.cpp \
    ../Tetris/src/NodeCalculatorImpl.cpp \
    ../Tetris/src/SingleThreadedNodeCalculator.cpp \
    ../Tetris/src/Threading.cpp \
    ../Tetris/src/Unicode.cpp \
    ../Tetris/src/Worker.cpp \
    ../Tetris/src/WorkerPool.cpp

HEADERS += \
    MainWindow.h \
    TetrisWidget.h \
    ../Tetris/include/Tetris/AISupport.h \
    ../Tetris/include/Tetris/Allocator.h \
    ../Tetris/include/Tetris/Assert.h \
    ../Tetris/include/Tetris/AutoPtrSupport.h \
    ../Tetris/include/Tetris/BlockFactory.h \
    ../Tetris/include/Tetris/BlockMover.h \
    ../Tetris/include/Tetris/BlockType.h \
    ../Tetris/include/Tetris/BlockTypes.h \
    ../Tetris/include/Tetris/ComputerPlayer.h \
    ../Tetris/include/Tetris/Config.h \
    ../Tetris/include/Tetris/Direction.h \
    ../Tetris/include/Tetris/Enum.h \
    ../Tetris/include/Tetris/Evaluator.h \
    ../Tetris/include/Tetris/ForwardDeclarations.h \
    ../Tetris/include/Tetris/GameOver.h \
    ../Tetris/include/Tetris/GameState.h \
    ../Tetris/include/Tetris/GameStateComparator.h \
    ../Tetris/include/Tetris/GameStateNode.h \
    ../Tetris/include/Tetris/GenericGrid.h \
    ../Tetris/include/Tetris/GenericGrid2.h \
    ../Tetris/include/Tetris/Logger.h \
    ../Tetris/include/Tetris/Logging.h \
    ../Tetris/include/Tetris/MakeString.h \
    ../Tetris/include/Tetris/MultithreadedNodeCalculator.h \
    ../Tetris/include/Tetris/NodeCalculator.h \
    ../Tetris/include/Tetris/NodeCalculatorImpl.h \
    ../Tetris/include/Tetris/NodePtr.h \
    ../Tetris/include/Tetris/SingleThreadedNodeCalculator.h \
    ../Tetris/include/Tetris/TypedWrapper.h \
    ../Tetris/include/Tetris/Unicode.h \
    ../Tetris/include/Tetris/Utilities.h \
    ../Tetris/include/Tetris/Worker.h \
    ../Tetris/include/Tetris/WorkerPool.h \
    ../Tetris/include/Tetris/Threading.h \
    ../Tetris/include/Tetris/Grid.h \
    ../Tetris/include/Tetris/Game.h \
    ../Tetris/include/Tetris/Tetris.h \
    ../Tetris/include/Tetris/Block.h \
    ../Tetris/include/Tetris/Gravity.h

OTHER_FILES += \
    CMakeLists.txt
