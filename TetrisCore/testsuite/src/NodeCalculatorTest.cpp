#include "NodeCalculatorTest.h"
#include "CppUnit/TestCaller.h"
#include "CppUnit/TestSuite.h"
#include "Tetris/NodeCalculator.h"
#include "Tetris/GameQualityEvaluator.h"
#include "Tetris/GameStateComparisonFunctor.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/GameState.h"
#include "Tetris/Block.h"
#include "Tetris/BlockTypes.h"
#include "Tetris/WorkerPool.h"
#include "Tetris/Assert.h"
#include "Tetris/Logging.h"
#include "Tetris/MakeString.h"
#include "Poco/Thread.h"
#include <iostream>


using namespace Tetris;


static const int cSleepTimeMs = 100;


NodeCalculatorTest::NodeCalculatorTest(const std::string & inName):
    CppUnit::TestCase(inName)
{
}


NodeCalculatorTest::~NodeCalculatorTest()
{
}


void NodeCalculatorTest::testNodeCalculator()
{
    test(Depth(4), Width(4), WorkerCount(1), TimeMs(2000));
    test(Depth(8), Width(5), WorkerCount(1), TimeMs(2000));
    test(Depth(4), Width(4), WorkerCount(4), TimeMs(2000));
    test(Depth(8), Width(5), WorkerCount(4), TimeMs(2000));
    test(Depth(4), Width(4), WorkerCount(4), TimeMs(4000));
    test(Depth(8), Width(5), WorkerCount(4), TimeMs(4000));
    test(Depth(4), Width(4), WorkerCount(8), TimeMs(4000));
    test(Depth(8), Width(5), WorkerCount(8), TimeMs(4000));
    test(Depth(8), Width(8), WorkerCount(8), TimeMs(8000));
}


void NodeCalculatorTest::test(Depth inDepth, Width inWidth, WorkerCount inWorkerCount, TimeMs inTimeMs)
{
    std::cout << std::endl << "    Depth: " << inDepth << ", Width: " << inWidth << ", WorkerCount: " << inWorkerCount << ", TimeMs: " << inTimeMs;
    std::auto_ptr<GameStateNode> rootNode = GameStateNode::CreateRootNode(20, 10);

    BlockTypes blockTypes;
    BlockType type = BlockType_Begin;
    for (int idx = 0; idx != inDepth; ++idx)
    {
        blockTypes.push_back(type);
        type = (type + 1) < BlockType_End ? (type + 1) : BlockType_Begin;
    }

    std::vector<int> widths(inDepth, inWidth);

    WorkerPool workerPool("NodeCalculatorTest", inWorkerCount);

    NodeCalculator nodeCalculator(rootNode->clone(), blockTypes, widths, rootNode->evaluator().clone(), workerPool);

    assert(nodeCalculator.getCurrentSearchDepth() == 0);
    assert(blockTypes.size() == widths.size());
    assert(nodeCalculator.getMaxSearchDepth() == blockTypes.size());
    assert(nodeCalculator.status() == NodeCalculator::Status_Nil);
    
    nodeCalculator.start();

    assert(nodeCalculator.status() == NodeCalculator::Status_Started);

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
                assert(nodeCalculator.status() == NodeCalculator::Status_Stopped ||
                       nodeCalculator.status() == NodeCalculator::Status_Finished);
                interrupted = true;
            }
        }
        Poco::Thread::sleep(10);
    }

    if (stopwatch.elapsed() / 1000 > inTimeMs)
    {
        int overtime = static_cast<int>(0.5 + stopwatch.elapsed() / 1000.0) - inTimeMs;
        assert(overtime < 500);
    }

    std::cout << (interrupted ? " -> Timeout" : " -> Succeeded");

    NodePtr resultPtr = nodeCalculator.result();
    GameStateNode & result(*resultPtr);
    assert(result.depth() == rootNode->depth() + 1);
    assert(result.state().originalBlock().type() == blockTypes[0]);
    assert(result.children().size() == 1);
    int depthDifference = result.endNode()->depth() - rootNode->depth();
    
    if (!interrupted)
    {
        assert(depthDifference == blockTypes.size());
        assert(nodeCalculator.getCurrentSearchDepth() == nodeCalculator.getMaxSearchDepth());
        assert(result.endNode()->state().originalBlock().type() == blockTypes.back());
    }
    else
    {
        std::cout << " (" << nodeCalculator.getCurrentSearchDepth() << "/" << nodeCalculator.getMaxSearchDepth() << ")";
        assert(nodeCalculator.getCurrentSearchDepth() < nodeCalculator.getMaxSearchDepth());
    }
    assert(result.endNode()->children().empty());
}


void NodeCalculatorTest::setUp()
{
}


void NodeCalculatorTest::tearDown()
{
}


CppUnit::Test * NodeCalculatorTest::suite()
{
    CppUnit::TestSuite * suite(new CppUnit::TestSuite("NodeCalculatorTest"));
	CppUnit_addTest(suite, NodeCalculatorTest, testNodeCalculator);
	return suite;
}
