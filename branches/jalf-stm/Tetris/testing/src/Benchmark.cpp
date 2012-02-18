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


struct Benchmark : public TetrisTest { };


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


void PerformBenchmark(unsigned inWidth, unsigned inDepth)
{
    UInt64 startTime = GetCurrentTimeMs();
    Worker worker("Main Worker");
    WorkerPool workerPool("Worker Pool", 8);
    std::cout << "Starting test with Depth=" << inDepth << " and Width=" << inWidth << std::endl;
    NodeCalculator nodeCalculator(GameState(20, 10),
                                  GetBlockTypes(inWidth),
                                  GetWidths(inWidth, inDepth),
                                  MakeTetrises::Instance(),
                                  worker,
                                  workerPool);

    nodeCalculator.start();
    unsigned c = 0;
    std::cout << std::endl;
    while (nodeCalculator.status() != NodeCalculator::Status_Finished)
    {
        Sleep(UInt64(50));
        std::cout << "\rNode count: " << nodeCalculator.getCurrentNodeCount() << "/" << nodeCalculator.getMaxNodeCount() << " (" << c++ << ")" << std::flush;
    }

    std::cout << "\rNode count: " << nodeCalculator.getCurrentNodeCount() << "/" << nodeCalculator.getMaxNodeCount() << std::endl;
    std::cout << "Finished in " << (GetCurrentTimeMs() - startTime) << "ms." << std::endl;
}


void Print(const std::exception & exc)
{
    std::cout << "Anticipated exception: " << exc.what() << std::endl;
}


} // anonymous namespace


TEST_F(Benchmark, Quick)
{
    try { PerformBenchmark(0, 0); } catch (const std::exception & exc) { Print(exc); }
    try { PerformBenchmark(0, 1); } catch (const std::exception & exc) { Print(exc); }
    try { PerformBenchmark(1, 0); } catch (const std::exception & exc) { Print(exc); }

    PerformBenchmark(1, 1);
    PerformBenchmark(2, 2);
    PerformBenchmark(4, 4);
    PerformBenchmark(6, 6);
}


} // namespace testing
