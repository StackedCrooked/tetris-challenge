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


int gTimeout = 15000;


} // anonymous namespace


namespace testing {

TEST_F(NodeCalculatorTest, Interrupt)
{
    int depth = 8;
    int width = 5;

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
    testInterrupt(Depth(1), Width(1), WorkerCount(1), TimeMs(10));
}


namespace {


enum {
    cOneBillion  = 1000 * 1000 * 1000,
    cOneMillion  = 1000 * 1000,
    cOneThousand = 1000
};


template<typename T>
std::string format(T n)
{
    std::stringstream ss;
    ss << n;
    return ss.str();/*
    std::string counter;
    if (n > cOneBillion)
    {
        ss << (double(n) / cOneBillion);
        counter = "G";
    }
    else if (n > cOneMillion)
    {
        ss << (double(n) / cOneMillion);
        counter = "M";
    }
    else
    {
        ss << (double(n) / cOneThousand);
        counter = "K";
    }

    std::string result = ss.str();
    std::string::size_type subLength = std::string::npos;
    std::string::size_type pointOffset = result.find(".");
    if (pointOffset != std::string::npos)
    {
        if (pointOffset + 1 < result.size())
        {
            subLength = pointOffset + 2;
        }
    }

    result = result.substr(0, subLength) + counter;
    return result;*/
}


}


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
    bool interrupted = false;
    int duration = 0;
    std::string message;
    while (nodeCalculator.status() != NodeCalculator::Status_Finished)
    {
        if (nodeCalculator.status() != NodeCalculator::Status_Stopped)
        {
            if (stopwatch.elapsed() > 1000 * inTimeMs)
            {
                nodeCalculator.stop();
                stopwatch.stop();
                ASSERT_TRUE(
                    nodeCalculator.status() == NodeCalculator::Status_Stopped ||
                    nodeCalculator.status() == NodeCalculator::Status_Finished);
                interrupted = true;
            }
        }

        duration = static_cast<int>(0.5 + stopwatch.elapsed() / 1000.0);


        std::stringstream ss;
        ss << "\r"
           << "W/D: "         << inWidth << "/" << inDepth
           << ". Duration: "
           << std::setw(2) << std::setfill(' ')
           << int(0.5 + duration/1000.0)
           << "/"
           << int(0.5 + inTimeMs/1000.0) << "s"
           << ". Workers: "
           << std::setw(2) << std::setfill(' ') << inWorkerCount
           << ". Result: "    << nodeCalculator.getCurrentSearchDepth() << "/" << nodeCalculator.getMaxSearchDepth()
           << ". Nodes: "
           << std::setw(4) << std::setfill(' ') << format(nodeCalculator.getCurrentNodeCount())
           << "/"
           << format(nodeCalculator.getMaxNodeCount());

        message = ss.str();
        std::cout << message << std::flush;

        if (stopwatch.elapsed() / 1000 > inTimeMs)
        {
            int overtime = duration - inTimeMs;
            ASSERT_TRUE(overtime < 500);
        }

        Futile::Sleep(10);
    }

    std::cout << message << std::endl;

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
