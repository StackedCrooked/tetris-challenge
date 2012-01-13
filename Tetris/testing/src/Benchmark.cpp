#include "TetrisTest.h"
#include "Tetris/Evaluator.h"
#include "Tetris/GameState.h"
#include "Tetris/NodeCalculator.h"
#include "Tetris/NodeCalculatorImpl.h"
#include "Futile/Threading.h"
#include <iostream>
#include <string>


namespace testing {


using namespace Futile;
using namespace Tetris;


namespace { // anonymous


BlockTypes GetBlockTypes(unsigned inDepth)
{
    BlockTypes blockTypes;
    int type = BlockType_Begin;
    for (unsigned idx = 0; idx != inDepth; ++idx)
    {
        blockTypes.push_back(BlockType(type));
        type = (type + 1) < int(BlockType_End) ? (type + 1) : int(BlockType_Begin);
    }
    return blockTypes;
}


std::vector<int> GetWidths(unsigned inDepth, int inWidth)
{
    return std::vector<int>(inDepth, inWidth);
}


} // anonymous namespace


class Benchmark : public TetrisTest
{
};


TEST_F(Benchmark, Quick)
{
    UInt64 startTime = GetCurrentTimeMs();
    Worker worker("Main Worker");
    WorkerPool workerPool("Worker Pool", 8);

    enum {
        Depth = 8,
        Width = 5
    };

    std::cout << "Starting test with Depth=" << Depth << " and Width=" << Width << std::endl;
    NodeCalculator nodeCalculator(GameState(20, 10),
                                  GetBlockTypes(Width),
                                  GetWidths(Width, Depth),
                                  MakeTetrises::Instance(),
                                  worker,
                                  workerPool);

    nodeCalculator.start();

    NodeCalculator::Status status = NodeCalculator::Status_Begin;
    while (status != NodeCalculator::Status_Finished)
    {
        if (status == NodeCalculator::Status_Error)
        {
            throw std::runtime_error(nodeCalculator.errorMessage());
        }
        Sleep(UInt64(1));

        NodeCalculator::Status newStatus = nodeCalculator.status();
        if (newStatus != status)
        {
            std::cout << "Status is now " << newStatus << std::endl;
        }

        status = newStatus;
    }

    std::cout << "Finished in " << (GetCurrentTimeMs() - startTime) << "ms." << std::endl;
}


} // namespace testing

