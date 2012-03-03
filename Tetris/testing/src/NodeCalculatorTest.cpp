#include "NodeCalculatorTest.h"
#include "Tetris/Block.h"
#include "Tetris/BlockTypes.h"
#include "Tetris/Evaluator.h"
#include "Tetris/GameStateComparator.h"
#include "Tetris/GameState.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/NodeCalculator.h"
#include "Futile/Assert.h"
#include "Futile/Logging.h"
#include "Futile/MainThreadImpl.h"
#include "Futile/MakeString.h"
#include "Futile/Stopwatch.h"
#include "Futile/WorkerPool.h"
#include "Futile/Threading.h"
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>



namespace testing {


using namespace Futile;
using namespace Tetris;


namespace { // anonymous


static const int cTimeout = 1000;


typedef std::pair<std::size_t, std::size_t> Pair;


const std::string cWorkers = "Threads";
const std::size_t cWorkersSize = cWorkers.size() + 2;

const std::string cSearchWidth = "Width x Depth";
const std::size_t cSearchWidthSize = cSearchWidth.size() + 2;

const std::string cDepthProgress= "Depth Progress";
const std::size_t cDepthProgressSize = cDepthProgress.size() + 2;

const std::string cElapsedTime = "Remaining Time";
const std::size_t cElapsedTimeSize = std::max<std::size_t>(cElapsedTime.size(), std::string("10000ms").size()) + 2;

const std::string cStatus = "Result";
const std::size_t cStatusSize = std::string("Timed out").size() + 2;


std::string header()
{
    std::stringstream ss;
    ss << std::setw(cWorkers.size()) << cWorkers
       << std::setw(cSearchWidthSize) << cSearchWidth
       << std::setw(cDepthProgressSize) << cDepthProgress
       << std::setw(cElapsedTimeSize) << cElapsedTime
       << std::setw(cStatusSize) << cStatus;
    return ss.str();
}


std::string format(std::size_t workerCount,
                  Pair widthAndDepth,
                  Pair time,
                  Pair depth)
{
    if (time.first > time.second)
        time.first = time.second;

    std::stringstream ss;
    ss << "\r"
       << std::setw(cWorkers.size()) << workerCount
       << std::setw(cSearchWidthSize) << (SS() << widthAndDepth.first << "x" << widthAndDepth.second).str()
       << std::setw(cDepthProgressSize) << (SS() << depth.first << "/" << depth.second).str()
       << std::setw(cElapsedTimeSize) << (SS() << time.first << "/" << time.second << "ms").str()
       << std::setw(cStatusSize) << std::right <<
          (time.first < time.second ? (depth.first < depth.second ? "Busy"
                                                                  : "OK"  )
            : "Timed out");
    return ss.str();
}


} // anonymous namespace

// TODO: This doesn't belong here. Move to a better location..
TEST_F(NodeCalculatorTest, BlockTypeIncrementation)
{
    Assert(increment(BlockType_I) == BlockType_J);
    Assert(increment(BlockType_J) == BlockType_L);
    Assert(increment(BlockType_L) == BlockType_O);
    Assert(increment(BlockType_O) == BlockType_S);
    Assert(increment(BlockType_S) == BlockType_T);
    Assert(increment(BlockType_T) == BlockType_Z);
    Assert(increment(BlockType_Z) == BlockType_I);
}


TEST_F(NodeCalculatorTest, SingleThreaded)
{
    testInterrupt(Depth(6), Width(6), WorkerCount(1), TimeMs(5000));
}


TEST_F(NodeCalculatorTest, Interrupt)
{

    std::cout << header() << std::endl;
    for (std::size_t w = 1; w <= 4; w *= 2)
    {
        for (std::size_t width = 1; width <= 4; width *= 2)
        {
            for (std::size_t depth = 1; depth <= 4; depth *= 2)
            {
                testInterrupt(Depth(depth), Width(width), WorkerCount(w), TimeMs(cTimeout));
            }
        }
    }
}


void NodeCalculatorTest::testInterrupt(Depth inDepth, Width inWidth, WorkerCount inWorkerCount, TimeMs inTimeMs)
{
    GameStateNode rootNode(GameState(20, 10), Balanced::Instance());

    BlockTypes blockTypes;
    int type = BlockType_Begin;
    for (int idx = 0; idx != inDepth; ++idx)
    {
        blockTypes.push_back(BlockType(type));
        type = (type + 1) < int(BlockType_End) ? (type + 1) : int(BlockType_Begin);
    }

    std::vector<int> widths(inDepth, inWidth);

    WorkerPool workerPool("Worker Pool", inWorkerCount);
    Worker mainWorker("Main Worker");


    NodeCalculator nodeCalculator(rootNode.gameState(), blockTypes, widths, rootNode.evaluator(), mainWorker, workerPool);

    ASSERT_TRUE(nodeCalculator.progress().current() == 0);
    ASSERT_TRUE(blockTypes.size() == widths.size());
    ASSERT_TRUE(std::size_t(nodeCalculator.progress().limit()) == blockTypes.size());
    ASSERT_TRUE(std::size_t(nodeCalculator.status()) == NodeCalculator::Status_Initial);

    nodeCalculator.start();

    ASSERT_TRUE(nodeCalculator.status() == NodeCalculator::Status_Starting);

    Futile::Stopwatch stopwatch;
    stopwatch.start();
    int duration = 0;
    while (nodeCalculator.status() != NodeCalculator::Status_Finished)
    {
        duration = stopwatch.elapsedMs();

        std::cout << format(inWorkerCount,
                            std::make_pair(inWidth, inDepth),
                            std::make_pair(duration, inTimeMs),
                            std::make_pair(nodeCalculator.progress().current(), nodeCalculator.progress().limit()));
        std::cout << std::flush;

        if (nodeCalculator.status() != NodeCalculator::Status_Stopping)
        {
            if (duration > inTimeMs)
            {
                nodeCalculator.stop();
                stopwatch.stop();
                ASSERT_TRUE(
                    nodeCalculator.status() == NodeCalculator::Status_Stopping ||
                    nodeCalculator.status() == NodeCalculator::Status_Finished);
                break;
            }
        }

        // Overtime should not exceed 500 ms.
        ASSERT_LT(duration, inTimeMs + 500);

        Futile::Sleep(10);
    }

    std::cout << format(inWorkerCount,
                        std::make_pair(inWidth, inDepth),
                        std::make_pair(duration, inTimeMs),
                        std::make_pair(nodeCalculator.progress().current(), nodeCalculator.progress().limit()));
    std::cout << std::endl;

    ASSERT_LE(nodeCalculator.progress().current(), nodeCalculator.progress().limit());

    std::vector<GameState> result = nodeCalculator.results();

    for (std::size_t idx = 0; idx + 1 < result.size(); ++idx)
    {
        Assert(result[idx].id() + 1 == result[idx + 1].id());
    }
}


TEST_F(NodeCalculatorTest, Destroy)
{
    for (size_t i = 0; i != 2; ++i)
    {
        for (size_t workerCount = 1; workerCount < 12; ++workerCount)
        {
            Worker mainWorker("Main Worker");
            WorkerPool workerPool("NodeCalculatorTest", workerCount);
            testDestroy(mainWorker, workerPool);
        }
    }
}


void NodeCalculatorTest::testDestroy(Worker & inMainWorker, WorkerPool & inWorkerPool)
{
    const int cWidth = 6;
    const int cDepth = 6;
    GameStateNode rootNode(GameState(20, 10), Balanced::Instance());

    BlockTypes blockTypes;
    int type = BlockType_Begin;
    for (int idx = 0; idx != cDepth; ++idx)
    {
        blockTypes.push_back(BlockType(type));
        type = (type + 1) < BlockType_End ? (type + 1) : int(BlockType_Begin);
    }

    std::vector<int> widths(cDepth, cWidth);

    NodeCalculator nodeCalculator(rootNode.gameState(), blockTypes, widths, rootNode.evaluator(), inMainWorker, inWorkerPool);

    ASSERT_TRUE(nodeCalculator.progress().current() == 0);
    ASSERT_TRUE(blockTypes.size() == widths.size());
    ASSERT_TRUE(std::size_t(nodeCalculator.progress().limit()) == blockTypes.size());
    ASSERT_TRUE(nodeCalculator.status() == NodeCalculator::Status_Initial);

    nodeCalculator.start();

    ASSERT_TRUE(nodeCalculator.status() >= NodeCalculator::Status_Starting);
    Futile::Sleep(10);
    ASSERT_TRUE(nodeCalculator.status() != NodeCalculator::Status_Error);
}


} // namespace testing
