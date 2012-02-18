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
    mQuitFlag(false),
    mStatus(0),
    mAllResults(AllResults(inEvaluator, inBlockTypes.size())),
    mBlockTypes(inBlockTypes),
    mWidths(inWidths),
    mMaxNodeCount(calculateMaxNodeCount(inWidths)),
    mEvaluator(inEvaluator),
    mMainWorker(inMainWorker),
    mWorkerPool(inWorkerPool)
{
    if (mBlockTypes.empty())
    {
        throw std::logic_error("Blocktypes must not be empty!");
    }

    if (mBlockTypes.size() != mWidths.size())
    {
        throw std::logic_error("Number of provided blocks does not equal number of provided widths.");
    }

    for (std::size_t idx = 0; idx < mWidths.size(); ++idx)
    {
        if (mWidths[idx] == 0)
        {
            throw std::logic_error("Width must be > 0.");
        }
    }

    Assert(!mNode->gameState().isGameOver());
    Assert(mNode->children().empty());
}


NodeCalculatorImpl::~NodeCalculatorImpl()
{
    mNode.reset();
}


void NodeCalculatorImpl::setQuitFlag()
{
    mQuitFlag = true;
}


bool NodeCalculatorImpl::getQuitFlag() const
{
    return mQuitFlag;
}


int NodeCalculatorImpl::getCurrentSearchDepth() const
{
    return stm::atomic<int>([&](stm::transaction & tx)
    {
        return mAllResults.depth(tx);
    });
}


int NodeCalculatorImpl::getMaxSearchDepth() const
{
    return mWidths.size();
}


unsigned NodeCalculatorImpl::getCurrentNodeCount() const
{
    return mCurrentNodeCount;
}


unsigned NodeCalculatorImpl::getMaxNodeCount() const
{
    return mMaxNodeCount;
}


std::vector<GameState> NodeCalculatorImpl::result() const
{
    Assert(status() == NodeCalculator::Status_Finished);
    return mResult;
}


int NodeCalculatorImpl::status() const
{
    return mStatus;
}


std::string NodeCalculatorImpl::errorMessage() const
{
    return mErrorMessage;
}


void NodeCalculatorImpl::setStatus(int inStatus)
{
    mStatus = inStatus;
}


void NodeCalculatorImpl::calculateResult(stm::transaction & tx)
{
    if (getQuitFlag())
    {
        return;
    }

    if (getCurrentSearchDepth() == 0 || !mAllResults.bestNode(tx))
    {
        Assert(mNode->endNode()->gameState().isGameOver());
        return;
    }

    // Backtrack the best end-node to its starting node.
    std::stack<NodePtr> results;
    NodePtr endNode = mAllResults.bestNode(tx);
    unsigned startId = mNode->gameState().id();
    unsigned currentId = endNode->gameState().id();
    while (currentId > startId + 1)
    {
        results.push(endNode);
        endNode = endNode->parent();
        unsigned parentId = endNode->gameState().id();
        mResult.push_back(endNode->gameState());
        Assert(currentId == parentId + 1);
        currentId = parentId;
    }

    std::reverse(mResult.begin(), mResult.end());
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
        stm::atomic([&](stm::transaction & tx) { calculateResult(tx); });
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
    mMainWorker.schedule(boost::bind(&NodeCalculatorImpl::startImpl, this));
    mStatus = NodeCalculator::Status_Started;
}


} // namespace Tetris
