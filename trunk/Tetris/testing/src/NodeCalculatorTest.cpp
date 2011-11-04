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


namespace { // anonymous


int gTimeout = 100;


} // anonymous namespace


namespace testing {

TEST_F(NodeCalculatorTest, Interrupt)
{
    int depth = 10;
    int width = 4;

    std::cout << std::endl;
    testInterrupt(Depth(depth), Width(width), WorkerCount(16), TimeMs(gTimeout));
    testInterrupt(Depth(depth), Width(width), WorkerCount(15), TimeMs(gTimeout));
    testInterrupt(Depth(depth), Width(width), WorkerCount(14), TimeMs(gTimeout));
    testInterrupt(Depth(depth), Width(width), WorkerCount(12), TimeMs(gTimeout));
    testInterrupt(Depth(depth), Width(width), WorkerCount(11), TimeMs(gTimeout));
    testInterrupt(Depth(depth), Width(width), WorkerCount(10), TimeMs(gTimeout));
    testInterrupt(Depth(depth), Width(width), WorkerCount(9), TimeMs(gTimeout));
    testInterrupt(Depth(depth), Width(width), WorkerCount(8), TimeMs(gTimeout));
    testInterrupt(Depth(depth), Width(width), WorkerCount(7), TimeMs(gTimeout));
    testInterrupt(Depth(depth), Width(width), WorkerCount(6), TimeMs(gTimeout));
    testInterrupt(Depth(depth), Width(width), WorkerCount(5), TimeMs(gTimeout));
    testInterrupt(Depth(depth), Width(width), WorkerCount(4), TimeMs(gTimeout));
    testInterrupt(Depth(depth), Width(width), WorkerCount(3), TimeMs(gTimeout));
    testInterrupt(Depth(depth), Width(width), WorkerCount(2), TimeMs(gTimeout));
    testInterrupt(Depth(depth), Width(width), WorkerCount(1), TimeMs(gTimeout));
    testInterrupt(Depth(1), Width(1), WorkerCount(1), TimeMs(10000));
}


void NodeCalculatorTest::testInterrupt(Depth inDepth, Width inWidth, WorkerCount inWorkerCount, TimeMs inTimeMs)
{
    std::cout << "    Depth: "     << std::setw(2) << std::setfill(' ') << inDepth
              << ", Width: "       << std::setw(2) << std::setfill(' ') << inWidth
              << ", WorkerCount: " << std::setw(2) << std::setfill(' ') << inWorkerCount
              << ", TimeMs: "      << std::setw(5) << std::setfill(' ') << inTimeMs;
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
    bool interrupted = false;
    while (nodeCalculator.status() != NodeCalculator::Status_Finished)
    {
        if (nodeCalculator.status() != NodeCalculator::Status_Stopped)
        {
            if (stopwatch.elapsed() > 1000 * inTimeMs)
            {
                nodeCalculator.stop();
                stopwatch.stop();
                ASSERT_TRUE(nodeCalculator.status() == NodeCalculator::Status_Stopped ||
                       nodeCalculator.status() == NodeCalculator::Status_Finished);
                interrupted = true;
            }
        }
        Poco::Thread::sleep(10);
    }

    int duration = static_cast<int>(0.5 + stopwatch.elapsed() / 1000.0);
    if (stopwatch.elapsed() / 1000 > inTimeMs)
    {
        int overtime = duration - inTimeMs;
        ASSERT_TRUE(overtime < 500);
    }



    std::cout
        << std::setw(30) << std::setfill(' ')
        << (interrupted ? (SS() << " -> Interrupted after " << duration << "ms") :
                          (SS() << " -> Succeeded in " << duration << "ms")).str()
        << ". Result: " << nodeCalculator.getCurrentSearchDepth() << "/" << nodeCalculator.getMaxSearchDepth()
        << ". Number of calculated nodes: "
            << std::setw(6) << std::setfill(' ') << std::pow(double(int(inWidth)), nodeCalculator.getCurrentSearchDepth())
            << " - "
            << std::setw(6) << std::setfill(' ') << std::pow(double(int(inWidth)), nodeCalculator.getCurrentSearchDepth() + 1)
            << "."
        << std::endl;



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
