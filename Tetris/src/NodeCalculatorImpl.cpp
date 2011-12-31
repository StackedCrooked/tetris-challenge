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
    mNode(NodePtr(new GameStateNode(inGameState, inEvaluator))),
    mResult(Result()),
    mQuitFlag(false),
    mTreeRowInfos(TreeRowInfos(inEvaluator, inBlockTypes.size())),
    mBlockTypes(inBlockTypes),
    mWidths(inWidths),
    mEvaluator(inEvaluator),
    mStatus(0),
    mErrorMessage(std::string()),
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


void NodeCalculatorImpl::setQuitFlag(stm::transaction & tx)
{
    mQuitFlag.open_rw(tx) = true;
}


bool NodeCalculatorImpl::getQuitFlag(stm::transaction & tx) const
{
    return mQuitFlag.open_r(tx);
}


int NodeCalculatorImpl::getCurrentSearchDepth(stm::transaction & tx) const
{
    return mTreeRowInfos.open_r(tx).depth(tx);
}


int NodeCalculatorImpl::getMaxSearchDepth() const
{
    return mWidths.size();
}


std::vector<GameState> NodeCalculatorImpl::result(stm::transaction & tx) const
{
    Assert(status(tx) == NodeCalculator::Status_Finished);
    return mResult.open_r(tx);
}


int NodeCalculatorImpl::status(stm::transaction & tx) const
{
    return mStatus.open_r(tx);
}


std::string NodeCalculatorImpl::errorMessage(stm::transaction & tx) const
{
    return mErrorMessage.open_r(tx);
}


void NodeCalculatorImpl::setStatus(stm::transaction & tx, int inStatus)
{
    mStatus.open_rw(tx) = inStatus;
}


void NodeCalculatorImpl::destroyInferiorChildren(stm::transaction & tx)
{
    std::size_t reachedDepth = getCurrentSearchDepth(tx);
    if (reachedDepth == 0)
    {
        Assert(getQuitFlag(tx));
        return;
    }

    // We use the 'best child' from this search depth.
    // The path between the start node and this best
    // child will be the list of precalculated nodes.
    NodePtr endNode = mTreeRowInfos.open_r(tx).bestNode(tx);
    Assert(endNode);
    if (endNode)
    {
        CarveBestPath(mNode, endNode);
        Assert(mNode->children().size() == 1);
    }
}


void NodeCalculatorImpl::calculateResult(stm::transaction & tx)
{
    if (getQuitFlag(tx))
    {
        return;
    }

    const TreeRowInfos & treeRowInfos = mTreeRowInfos.open_r(tx);
    if (getCurrentSearchDepth(tx) == 0 || !treeRowInfos.bestNode(tx))
    {
        Assert(mNode->endNode()->gameState().isGameOver());
        return;
    }

    // Backtrack the best end-node to its starting node.
    Result localResult;
    NodePtr endNode = treeRowInfos.bestNode(tx);

    unsigned startId = mNode->id();
    unsigned currentId = endNode->gameState().id();
    while (currentId > startId + 1)
    {
        endNode = endNode->parent();
        unsigned parentId = endNode->gameState().id();
        localResult.push_back(endNode->gameState());
        Assert(currentId == parentId + 1);
        currentId = parentId;
    }

    std::reverse(localResult.begin(), localResult.end());
    mResult.open_rw(tx) = localResult;
}


void NodeCalculatorImpl::stop(stm::transaction & tx)
{
    switch (status(tx))
    {
        case NodeCalculator::Status_Started:
        case NodeCalculator::Status_Working:
        {
            setStatus(tx, NodeCalculator::Status_Stopped);
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


void NodeCalculatorImpl::startImpl(stm::transaction & tx)
{
    // Thread entry point has try/catch block
    try
    {
        setStatus(tx, NodeCalculator::Status_Working);
        populate();
        calculateResult(tx);
        setStatus(tx, NodeCalculator::Status_Finished);
    }
    catch (const std::exception & inException)
    {
        mErrorMessage.open_rw(tx) = inException.what();
        setStatus(tx, NodeCalculator::Status_Error);
    }
}


void NodeCalculatorImpl::start(stm::transaction & tx)
{
    Assert(mStatus.open_r(tx) == NodeCalculator::Status_Nil);
    mMainWorker.schedule(boost::bind(&NodeCalculatorImpl::startImpl, this, boost::ref(tx)));
    mStatus.open_rw(tx) = NodeCalculator::Status_Started;
}


} // namespace Tetris
