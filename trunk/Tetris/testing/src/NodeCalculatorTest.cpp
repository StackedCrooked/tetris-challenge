#include "Poco/Foundation.h"
#include "NodeCalculatorTest.h"
#include "Tetris/NodeCalculator.h"
#include "Tetris/Evaluator.h"
#include "Tetris/GameStateComparator.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/GameState.h"
#include "Tetris/Block.h"
#include "Tetris/BlockTypes.h"
#include "Futile/WorkerPool.h"
#include "Futile/Assert.h"
#include "Futile/Logging.h"
#include "Futile/MakeString.h"
#include "Poco/Thread.h"
#include <iomanip>
#include <iostream>
#include <sstream>


namespace { // anonymous


int gTimeout = 5000;


} // anonymous namespace


namespace testing {


TEST_F(NodeCalculatorTest, Interrupt)
{
    std::cout << std::endl;
    for (std::size_t depth = 6; depth <= 8; ++depth)
    {
        for (std::size_t width = 4; width <= 6; ++width)
        {
            testInterrupt(Depth(depth), Width(width), WorkerCount(8), TimeMs(gTimeout));
            testInterrupt(Depth(depth), Width(width), WorkerCount(4), TimeMs(gTimeout));
            testInterrupt(Depth(depth), Width(width), WorkerCount(2), TimeMs(gTimeout));
        }
    }
}


namespace { // anonymous


typedef std::pair<std::size_t, std::size_t> Pair;
std::string format(std::size_t workerCount,
                  const Pair & widthAndDepth,
                  const Pair & time,
                  const Pair & depth,
                  const Pair & nodeCount)
{
    std::stringstream ss;
    ss << "\r"
       << "Workers: " << workerCount
       << " Width: " << widthAndDepth.first
       << " Depth: " << depth.first << "/" << depth.second
       << " Nodes: " << std::setw(15) << (SS() << nodeCount.first << "/" << nodeCount.second).str()
       << " Duration: " << std::setw(4) << time.first << "/" << std::setw(4) << time.second << "ms";
    return ss.str();
}


} // anonymous namespace


void NodeCalculatorTest::testInterrupt(Depth inDepth, Width inWidth, WorkerCount inWorkerCount, TimeMs inTimeMs)
{
    std::auto_ptr<GameStateNode> rootNode = GameStateNode::CreateRootNode(20, 10);

    BlockTypes blockTypes;
    BlockType type = BlockType_Begin;
    for (int idx = 0; idx != inDepth; ++idx)
    {
        blockTypes.push_back(type);
        type = (type + 1) < BlockType_End ? (type + 1) : BlockType_Begin;
    }

    std::vector<int> widths(inDepth, inWidth);

    WorkerPool workerPool("Worker Pool", inWorkerCount);
    Worker mainWorker("Main Worker");

    NodeCalculator nodeCalculator(rootNode->clone(), blockTypes, widths, rootNode->evaluator(), mainWorker, workerPool);

    ASSERT_TRUE(nodeCalculator.getCurrentSearchDepth() == 0);
    ASSERT_TRUE(blockTypes.size() == widths.size());
    ASSERT_TRUE(std::size_t(nodeCalculator.getMaxSearchDepth()) == blockTypes.size());
    ASSERT_TRUE(std::size_t(nodeCalculator.status()) == NodeCalculator::Status_Nil);

    nodeCalculator.start();

    ASSERT_TRUE(nodeCalculator.status() == NodeCalculator::Status_Started);

    Poco::Stopwatch stopwatch;
    stopwatch.start();
    int duration = 0;
    while (nodeCalculator.status() != NodeCalculator::Status_Finished)
    {

        duration = static_cast<int>(0.5 + stopwatch.elapsed() / 1000.0);

        std::cout << format(inWorkerCount,
                            std::make_pair(inWidth, inDepth),
                            std::make_pair(duration, inTimeMs),
                            std::make_pair(nodeCalculator.getCurrentSearchDepth(), nodeCalculator.getMaxSearchDepth()),
                            std::make_pair(nodeCalculator.getCurrentNodeCount(), nodeCalculator.getMaxNodeCount()));
        std::cout << std::flush;

        if (nodeCalculator.status() != NodeCalculator::Status_Stopped)
        {
            if (duration > inTimeMs)
            {
                nodeCalculator.stop();
                stopwatch.stop();
                ASSERT_TRUE(
                    nodeCalculator.status() == NodeCalculator::Status_Stopped ||
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
                        std::make_pair(nodeCalculator.getCurrentSearchDepth(), nodeCalculator.getMaxSearchDepth()),
                        std::make_pair(nodeCalculator.getCurrentNodeCount(), nodeCalculator.getMaxNodeCount()));
    std::cout << std::endl;

    ASSERT_LE(nodeCalculator.getCurrentSearchDepth(), nodeCalculator.getMaxSearchDepth());

    if(nodeCalculator.getCurrentSearchDepth() < nodeCalculator.getMaxSearchDepth())
    {
        ASSERT_LT(nodeCalculator.getCurrentNodeCount(), nodeCalculator.getMaxNodeCount());
    }

    if (nodeCalculator.getCurrentSearchDepth() == nodeCalculator.getMaxSearchDepth())
    {
        ASSERT_EQ(nodeCalculator.getCurrentNodeCount(), nodeCalculator.getMaxNodeCount());
    }

    NodePtr resultPtr = nodeCalculator.result();
    GameStateNode & result(*resultPtr);
    ASSERT_TRUE(result.depth() == rootNode->depth() + 1);
    ASSERT_TRUE(result.gameState().originalBlock().type() == blockTypes[0]);

    if (inDepth > 1)
    {
        ASSERT_TRUE(result.children().size() == 1);
    }
    else
    {
        ASSERT_TRUE(result.children().empty());
    }

    ASSERT_TRUE(result.endNode()->children().empty());
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
    std::auto_ptr<GameStateNode> rootNode = GameStateNode::CreateRootNode(20, 10);

    BlockTypes blockTypes;
    BlockType type = BlockType_Begin;
    for (int idx = 0; idx != cDepth; ++idx)
    {
        blockTypes.push_back(type);
        type = (type + 1) < BlockType_End ? (type + 1) : BlockType_Begin;
    }

    std::vector<int> widths(cDepth, cWidth);

    NodeCalculator nodeCalculator(rootNode->clone(), blockTypes, widths, rootNode->evaluator(), inMainWorker, inWorkerPool);

    ASSERT_TRUE(nodeCalculator.getCurrentSearchDepth() == 0);
    ASSERT_TRUE(blockTypes.size() == widths.size());
    ASSERT_TRUE(std::size_t(nodeCalculator.getMaxSearchDepth()) == blockTypes.size());
    ASSERT_TRUE(nodeCalculator.status() == NodeCalculator::Status_Nil);

    nodeCalculator.start();

    ASSERT_TRUE(nodeCalculator.status() >= NodeCalculator::Status_Started);
    Poco::Thread::sleep(10);
    ASSERT_TRUE(nodeCalculator.status() != NodeCalculator::Status_Error);
}


} // namespace testing
