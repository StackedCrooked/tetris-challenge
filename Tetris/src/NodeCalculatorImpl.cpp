#include "Tetris/Config.h"
#include "Tetris/NodeCalculatorImpl.h"
#include "Tetris/NodeCalculator.h"
#include "Tetris/AISupport.h"
#include "Tetris/Evaluator.h"
#include "Tetris/GameStateComparator.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/GameState.h"
#include "Tetris/Block.h"
#include "Tetris/BlockTypes.h"
#include "Futile/WorkerPool.h"
#include "Futile/Logging.h"
#include "Futile/MakeString.h"
#include "Futile/Threading.h"
#include <boost/shared_ptr.hpp>
#include <memory>
#include <stack>
#include <stdexcept>


namespace Tetris {


using namespace Futile;


NodeCalculatorImpl::NodeCalculatorImpl(std::auto_ptr<GameStateNode> inNode,
                                       const BlockTypes & inBlockTypes,
                                       const std::vector<int> & inWidths,
                                       const Evaluator & inEvaluator,
                                       Worker & inMainWorker,
                                       WorkerPool & inWorkerPool) :
    mNode(inNode.release()),
    mResult(),
    mNodeMutex(),
    mQuitFlag(false),
    mQuitFlagMutex(),
    mTreeRowInfos(inEvaluator, inBlockTypes.size()),
    mBlockTypes(inBlockTypes),
    mWidths(inWidths),
    mEvaluator(inEvaluator),
    mStatus(0),
    mStatusMutex(),
    mMainWorker(inMainWorker),
    mWorkerPool(inWorkerPool)
{
    Assert(!mNode->gameState().isGameOver());
    Assert(mNode->children().empty());
}


NodeCalculatorImpl::~NodeCalculatorImpl()
{
    mNode.reset();
}


void NodeCalculatorImpl::setQuitFlag()
{
    ScopedLock lock(mQuitFlagMutex);
    mQuitFlag = true;
}


bool NodeCalculatorImpl::getQuitFlag() const
{
    ScopedLock lock(mQuitFlagMutex);
    return mQuitFlag;
}


int NodeCalculatorImpl::getCurrentSearchDepth() const
{
    return mTreeRowInfos.depth();
}


int NodeCalculatorImpl::getMaxSearchDepth() const
{
    return mWidths.size();
}


NodePtr NodeCalculatorImpl::result() const
{
    Assert(status() == NodeCalculator::Status_Finished);
    ScopedLock lock(mNodeMutex);
    return mResult;
}


int NodeCalculatorImpl::status() const
{
    ScopedLock lock(mStatusMutex);
    return mStatus;
}


std::string NodeCalculatorImpl::errorMessage() const
{
    return mErrorMessage;
}


void NodeCalculatorImpl::setStatus(int inStatus)
{
    ScopedLock lock(mStatusMutex);
    mStatus = inStatus;
}


void NodeCalculatorImpl::populateNodesRecursively(
    NodePtr ioNode,
    const BlockTypes & inBlockTypes,
    const std::vector<int> & inWidths,
    std::size_t inIndex,
    std::size_t inMaxIndex)
{

    // We want to at least perform a search of depth 4.
    if (inIndex >= 4)
    {
        boost::this_thread::interruption_point();
    }

    //
    // Check stop conditions
    //
    if (inIndex > inMaxIndex || inIndex >= inBlockTypes.size())
    {
        return;
    }


    if (ioNode->gameState().isGameOver())
    {
        // GameOver state has no children.
        return;
    }


    //
    // Generate the child nodes.
    //
    // It is possible that the nodes were already generated at this depth.
    // If that is the case then we immediately jump to the recursive call below.
    //
    ChildNodes generatedChildNodes = ioNode->children();
    if (generatedChildNodes.empty())
    {
        generatedChildNodes = ChildNodes(GameStateComparator());
        GenerateOffspring(ioNode, inBlockTypes[inIndex], mEvaluator, generatedChildNodes);

        int count = 0;
        ChildNodes::iterator it = generatedChildNodes.begin(), end = generatedChildNodes.end();
        while (count < inWidths[inIndex] && it != end)
        {
            ioNode->addChild(*it);
            ++count;
            ++it;
        }
        Assert(count >= 1);
        mTreeRowInfos.registerNode(*ioNode->children().begin());
    }


    //
    // Recursive call on each child node.
    //
    if (inIndex < inMaxIndex)
    {
        for (ChildNodes::iterator it = generatedChildNodes.begin(); it != generatedChildNodes.end(); ++it)
        {
            NodePtr child = *it;
            populateNodesRecursively(child, inBlockTypes, inWidths, inIndex + 1, inMaxIndex);
        }
    }
}


void NodeCalculatorImpl::destroyInferiorChildren()
{
    std::size_t reachedDepth = getCurrentSearchDepth();
    if (reachedDepth == 0)
    {
        Assert(getQuitFlag());
        return;
    }

    // We use the 'best child' from this search depth.
    // The path between the start node and this best
    // child will be the list of precalculated nodes.
    ScopedLock nodeLock(mNodeMutex);
    NodePtr endNode = mTreeRowInfos.bestNode();
    Assert(endNode);
    if (endNode)
    {
        CarveBestPath(mNode, endNode);
        Assert(mNode->children().size() == 1);
    }
}


void NodeCalculatorImpl::calculateResult()
{
    if (getQuitFlag())
    {
        return;
    }

    if (getCurrentSearchDepth() == 0 || !mTreeRowInfos.bestNode())
    {
        return;
    }

    ScopedLock lock(mNodeMutex);

    // Backtrack the best end-node to its starting node.
    std::stack<NodePtr> results;
    NodePtr endNode = mTreeRowInfos.bestNode();
    while (endNode->depth() > mNode->depth())
    {
        results.push(endNode);
        endNode = endNode->parent();
    }

    NodePtr currentNode;
    NodePtr currentParent = mNode;
    while (!results.empty())
    {
        currentNode = results.top();
        NodePtr copy(new GameStateNode(currentParent, new GameState(currentNode->gameState()), currentNode->evaluator()));
        if (!mResult)
        {
            mResult = copy;
        }
        else
        {
            Assert(mResult->endNode()->depth() + 1 == copy->depth());
            mResult->endNode()->addChild(copy);
        }
        currentParent = copy;
        results.pop();
    }
}


void NodeCalculatorImpl::stop()
{
    if (status() == NodeCalculator::Status_Started || status() == NodeCalculator::Status_Working)
    {
        setStatus(NodeCalculator::Status_Stopped);
        Assert(mMainWorker.size() <= 1);
        mMainWorker.interruptAndClearQueue();
        mWorkerPool.interruptAndClearQueue();
    }
}


void NodeCalculatorImpl::startImpl()
{
    // Thread entry point has try/catch block
    try
    {
        setStatus(NodeCalculator::Status_Working);
        populate();
        calculateResult();
        setStatus(NodeCalculator::Status_Finished);
    }
    catch (const std::exception & inException)
    {
        mErrorMessage = inException.what();
        setStatus(NodeCalculator::Status_Error);
    }
}


void NodeCalculatorImpl::start()
{
    ScopedLock lock(mStatusMutex);
    Assert(mStatus == NodeCalculator::Status_Nil);
    mMainWorker.schedule(boost::bind(&NodeCalculatorImpl::startImpl, this));
    mStatus = NodeCalculator::Status_Started;
}


} // namespace Tetris
