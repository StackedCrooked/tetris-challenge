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
#ifdef _DEBUG
    int depth = 5;
    int width = 5;
#else
    int depth = 6;
    int width = 6;
#endif

    test(Depth(depth), Width(width), WorkerCount(6), TimeMs(10000));
    test(Depth(depth), Width(width), WorkerCount(4), TimeMs(10000));
    test(Depth(depth), Width(width), WorkerCount(3), TimeMs(10000));
    test(Depth(depth), Width(width), WorkerCount(2), TimeMs(10000));
    test(Depth(depth), Width(width), WorkerCount(1), TimeMs(10000));
    test(Depth(1), Width(1), WorkerCount(1), TimeMs(10000));
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

    Assert(nodeCalculator.getCurrentSearchDepth() == 0);
    Assert(blockTypes.size() == widths.size());
    Assert(nodeCalculator.getMaxSearchDepth() == blockTypes.size());
    Assert(nodeCalculator.status() == NodeCalculator::Status_Nil);
    
    nodeCalculator.start();

    Assert(nodeCalculator.status() == NodeCalculator::Status_Started);

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
                Assert(nodeCalculator.status() == NodeCalculator::Status_Stopped ||
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
        Assert(overtime < 500);
    }

    std::cout << (interrupted ? " -> Timeout" : std::string(MakeString() << " -> Succeeded in " << duration << "ms"));

    NodePtr resultPtr = nodeCalculator.result();
    GameStateNode & result(*resultPtr);
    Assert(result.depth() == rootNode->depth() + 1);
    Assert(result.state().originalBlock().type() == blockTypes[0]);

	if (inDepth > 1)
	{
		Assert(result.children().size() == 1);
	}
	else
	{
		Assert(result.children().empty());
	}

    int depthDifference = result.endNode()->depth() - rootNode->depth();
    
    if (!interrupted)
    {
        Assert(depthDifference == blockTypes.size());
        Assert(nodeCalculator.getCurrentSearchDepth() == nodeCalculator.getMaxSearchDepth());
        Assert(result.endNode()->state().originalBlock().type() == blockTypes.back());
    }
    else
    {
        std::cout << " (" << nodeCalculator.getCurrentSearchDepth() << "/" << nodeCalculator.getMaxSearchDepth() << ")";
        Assert(nodeCalculator.getCurrentSearchDepth() < nodeCalculator.getMaxSearchDepth());
    }
    Assert(result.endNode()->children().empty());
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
