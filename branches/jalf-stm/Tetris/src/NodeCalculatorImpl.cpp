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
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <memory>
#include <stack>
#include <stdexcept>


namespace Tetris {


using namespace Futile;


namespace { // anonymous


std::size_t calculateMaxNodeCount(const std::vector<int> & inWidths)
{
    std::vector<std::size_t> results;
    for (std::size_t idx = 0; idx < inWidths.size(); ++idx)
    {
        if (results.empty())
        {
            results.push_back(inWidths[idx]);
        }
        else
        {
            results.push_back(results.back() * inWidths[idx]);
        }
    }

    std::size_t result = 0;
    for (std::size_t idx = 0; idx < results.size(); ++idx)
    {
        result += results[idx];
    }
    return result;
}


} // anonymous namespace


NodeCalculatorImpl::NodeCalculatorImpl(const GameState & inGameState,
                                       const BlockTypes & inBlockTypes,
                                       const std::vector<int> & inWidths,
                                       const Evaluator & inEvaluator,
                                       Worker & inMainWorker,
                                       WorkerPool & inWorkerPool) :
    mNode(new GameStateNode(inGameState, inEvaluator)),
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


void OnPopulated(const GameState &)
{
}


void NodeCalculatorImpl::populateNodesRecursively(
    const GameState & inGameState,
    const BlockTypes & inBlockTypes,
    const std::vector<int> & inWidths,
    std::size_t inIndex,
    std::size_t inMaxIndex)
{
    CalculateNodes(inGameState, mEvaluator, inBlockTypes, inWidths, Progress(inIndex, inMaxIndex), boost::bind(&OnPopulated, _1));
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
        LogWarning("No results yet.");
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
        NodePtr copy(new GameStateNode(currentParent, GameState(currentNode->gameState()), currentNode->evaluator()));
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
    switch (status())
    {
        case NodeCalculator::Status_Started:
        case NodeCalculator::Status_Working:
        {
            setStatus(NodeCalculator::Status_Stopped);
            Assert(mMainWorker.size() <= 1);
            mMainWorker.interruptAndClearQueue();
            mWorkerPool.interruptAndClearQueue();
        }
        default:
        {
            // No action required.
        }
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
