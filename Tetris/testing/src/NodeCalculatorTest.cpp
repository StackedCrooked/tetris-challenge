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


namespace testing {
namespace { // anonymous


typedef std::pair<std::size_t, std::size_t> Pair;


const std::string cWorkers = "Threads";
const std::size_t cWorkersSize = cWorkers.size() + 2;

const std::string cSearchWidth = "Width x Depth";
const std::size_t cSearchWidthSize = cSearchWidth.size() + 2;

const std::string cSearchDepth= "Search Depth";
const std::size_t cSearchDepthSize = cSearchDepth.size() + 2;

const std::string cNodes = "Nodes";
const std::size_t cNodesSize = std::string("100%").size() + 2;

const std::string cTime = "Time";
const std::size_t cTimeSize = std::string("10000/10000").size() + 2;

const std::string cStatus = "Result";
const std::size_t cStatusSize = std::string("Timed out").size() + 2;


std::string header()
{
    std::stringstream ss;
    ss << std::setw(cWorkers.size()) << cWorkers
       << std::setw(cSearchWidthSize) << cSearchWidth
       << std::setw(cSearchDepthSize) << cSearchDepth
       << std::setw(cNodesSize) << cNodes
       << std::setw(cTimeSize) << cTime
       << std::setw(cStatusSize) << cStatus;
    return ss.str();
}


std::string format(std::size_t workerCount,
                  const Pair & widthAndDepth,
                  const Pair & time,
                  const Pair & depth,
                  const Pair & nodes)
{
    std::stringstream ss;
    ss << "\r"
       << std::setw(cWorkers.size()) << workerCount
       << std::setw(cSearchWidthSize) << (SS() << widthAndDepth.first << "x" << widthAndDepth.second).str()
       << std::setw(cSearchDepthSize) << (SS() << depth.first << "/" << depth.second).str()
       << std::setw(cNodesSize) << (SS() << int(0.5 + 100 * double(nodes.first) / double(nodes.second)) << "%").str()
       << std::setw(cTimeSize) << (SS() << int(0.5 + (time.second - time.first) / 1000.0) << "s").str()
       << std::setw(cStatusSize) << std::right <<
          (time.first < time.second ? (depth.first < depth.second ? "Busy"
                                                                  : "OK"  )
            : "Timed out");
    return ss.str();
}


int gTimeout = 10000;


} // anonymous namespace


TEST_F(NodeCalculatorTest, Interrupt)
{

    std::cout << header() << std::endl;
    for (std::size_t w = 2; w <= 8; w *= 2)
    {
        for (std::size_t width = 6; width <= 8; width += 2)
        {
            for (std::size_t depth = 6; depth <= 8; depth += 2)
            {
                testInterrupt(Depth(depth), Width(width), WorkerCount(w), TimeMs(gTimeout));
                testInterrupt(Depth(depth), Width(width), WorkerCount(w), TimeMs(gTimeout));
                testInterrupt(Depth(depth), Width(width), WorkerCount(w), TimeMs(gTimeout));
            }
        }
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
