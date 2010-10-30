include_directories(include)


add_library(TetrisCore
    src/AISupport.cpp 
    src/Block.cpp 
    src/BlockFactory.cpp 
    src/BlockMover.cpp 
    src/BlockType.cpp 
    src/ComputerPlayer.cpp 
    src/Game.cpp 
    src/GameQualityEvaluator.cpp 
    src/GameState.cpp 
    src/GameStateComparisonFunctor.cpp 
    src/GameStateNode.cpp 
    src/Gravity.cpp 
    src/Logger.cpp 
    src/Logging.cpp 
    src/MultiThreadedNodeCalculator.cpp 
    src/NodeCalculator.cpp 
    src/NodeCalculatorImpl.cpp 
    src/SingleThreadedNodeCalculator.cpp 
    src/Unicode.cpp 
    src/Worker.cpp
    src/WorkerPool.cpp)

if(APPLE)
set_target_properties(TetrisCore PROPERTIES COMPILE_FLAGS "-arch i386")
endif(APPLE)
set_target_properties(TetrisCore PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} -Wall")

