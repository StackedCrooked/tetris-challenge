include_directories(include)


add_library(TetrisCore
    src/AISupport.cpp 
    src/Block.cpp 
    src/BlockFactory.cpp 
    src/BlockMover.cpp 
    src/BlockType.cpp 
    src/ComputerPlayer.cpp 
    src/Game.cpp 
    src/GenericGrid.cpp 
    src/Evaluator.cpp 
    src/GameState.cpp 
    src/GameStateComparator.cpp 
    src/GameStateNode.cpp 
    src/Gravity.cpp 
    src/Logger.cpp 
    src/Logging.cpp 
    src/MultiThreadedNodeCalculator.cpp 
    src/NodeCalculator.cpp 
    src/NodeCalculatorImpl.cpp 
    src/SingleThreadedNodeCalculator.cpp 
    src/Threading.cpp 
    src/Unicode.cpp 
    src/Worker.cpp
    src/WorkerPool.cpp)

set_target_properties(TetrisCore PROPERTIES COMPILE_FLAGS ${CXX_FLAGS})

