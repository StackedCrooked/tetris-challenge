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
    std::auto_ptr<GameStateNode> rootNode = GameStateNode::CreateRootNode(20, 10);

    BlockTypes blockTypes;
    blockTypes.push_back(BlockType_I);
    blockTypes.push_back(BlockType_J);
    blockTypes.push_back(BlockType_T);
    blockTypes.push_back(BlockType_S);

    std::vector<int> widths(4, 4);

    WorkerPool workerPool("NodeCalculatorTest", 2);

    NodeCalculator nodeCalculator(rootNode->clone(), blockTypes, widths, rootNode->evaluator().clone(), workerPool);

    assert(nodeCalculator.getCurrentSearchDepth() == 0);
    assert(blockTypes.size() == widths.size());
    assert(nodeCalculator.getMaxSearchDepth() == blockTypes.size());
    assert(nodeCalculator.status() == NodeCalculator::Status_Nil);
    
    nodeCalculator.start();

    assert(nodeCalculator.status() == NodeCalculator::Status_Started);

    while (nodeCalculator.status() != NodeCalculator::Status_Finished)
    {
        Poco::Thread::sleep(1);
    }

    NodePtr resultPtr = nodeCalculator.result();
    GameStateNode & result(*resultPtr);
    assert(result.depth() == rootNode->depth() + 1);
    assert(result.state().originalBlock().type() == blockTypes[0]);
    assert(result.children().size() == 1);
    int depthDifference = result.endNode()->depth() - rootNode->depth();
    assert(depthDifference == blockTypes.size());
    assert(result.endNode()->state().originalBlock().type() == blockTypes.back());
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
