#include "Tetris/NodeCalculatorImpl.h"
#include "Tetris/NodeCalculator.h"
#include "Tetris/AISupport.h"
#include "Tetris/Evaluator.h"
#include "Tetris/GameStateComparator.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/GameState.h"
#include "Tetris/Block.h"
#include "Tetris/BlockTypes.h"
#include "Futile/Logging.h"
#include "Futile/MakeString.h"
#include "Futile/STMSupport.h"
#include "Futile/Threading.h"
#include "Futile/WorkerPool.h"
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <memory>
#include <stack>
#include <stdexcept>


namespace Tetris {


using namespace Futile;


namespace { // anonymous

#if 0 // disabled code
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
#endif


const GameState & NotGameOver(const GameState & inGameState)
{
    if (inGameState.isGameOver())
    {
        throw GameOver(inGameState);
    }

    return inGameState;
}


} // anonymous namespace


NodeCalculatorImpl::NodeCalculatorImpl(const GameState & inGameState,
                                       const BlockTypes & inBlockTypes,
                                       const std::vector<int> & inWidths,
                                       const Evaluator & inEvaluator,
                                       Worker & inMainWorker,
                                       WorkerPool & inWorkerPool) :
    mRootNode(new GameStateNode(NotGameOver(inGameState), inEvaluator)),
    mResults(GameStateList()),
    mVerticalResults(VerticalResults(inBlockTypes.size())),
    cBlockTypes(inBlockTypes),
    cWidths(inWidths),
    cEvaluator(inEvaluator),
    mMainWorker(inMainWorker),
    mWorkerPool(inWorkerPool),
    mQuitFlag(new bool(false)),
    mStatus(new int(0))
{
    if (cBlockTypes.empty())
    {
        throw std::logic_error("Blocktypes must not be empty!");
    }

    if (cBlockTypes.size() != cWidths.size())
    {
        throw std::logic_error("Number of provided blocks does not equal number of provided widths.");
    }

    for (std::size_t idx = 0; idx < cWidths.size(); ++idx)
    {
        if (cWidths[idx] == 0)
        {
            throw std::logic_error("Width must be > 0.");
        }
    }

    Assert(!mRootNode->gameState().isGameOver());
    Assert(mRootNode->children().empty());
}


NodeCalculatorImpl::~NodeCalculatorImpl()
{
}


void NodeCalculatorImpl::setQuitFlag()
{
    mQuitFlag.set(true);
}


bool NodeCalculatorImpl::getQuitFlag() const
{
    return mQuitFlag.get();
}


int NodeCalculatorImpl::getCurrentSearchDepth() const
{
    return stm::atomic<int>([&](stm::transaction & tx)
    {
        return mVerticalResults.depth(tx);
    });
}


int NodeCalculatorImpl::getMaxSearchDepth() const
{
    return cWidths.size();
}


std::vector<GameState> NodeCalculatorImpl::result() const
{
    if (mStatus.get() == NodeCalculator::Status_Error)
    {
        throw std::runtime_error(mErrorMessage.c_str());
    }
    return Futile::STM::get(mResults).get();
}


int NodeCalculatorImpl::status() const
{
    return mStatus.get();
}


std::string NodeCalculatorImpl::errorMessage() const
{
    return mErrorMessage;
}


void NodeCalculatorImpl::setStatus(int inStatus)
{
    mStatus.set(inStatus);
}


void NodeCalculatorImpl::calculateResult(stm::transaction & tx)
{
    if (getQuitFlag())
    {
        return;
    }

    if (getCurrentSearchDepth() == 0 || !mVerticalResults.bestNode(tx))
    {
        return;
    }

    // Backtrack the best end-node to its starting node.
    GameStateList result;
    NodePtr endNode = mVerticalResults.bestNode(tx);
    while (endNode != this->mRootNode)
    {
        result.push_front(endNode->gameState());
        endNode = endNode->parent();
    }

    #ifndef NDEBUG
    auto oldSize = mResults.open_r(tx).size();
    auto newSize = result.size();
    Assert(newSize >= oldSize);
    #endif

    mResults.open_rw(tx) = result;
}


void NodeCalculatorImpl::stop()
{
    switch (status())
    {
        case NodeCalculator::Status_Working:
        {
            setStatus(NodeCalculator::Status_Stopping);
            Assert(mMainWorker.size() <= 1);
            mMainWorker.interruptAndClearQueue();
            mWorkerPool.interruptAndClearQueue();
            setStatus(NodeCalculator::Status_Finished);
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
        setStatus(NodeCalculator::Status_Finished);
    }
    catch (const SetFinishedException & inException)
    {
        LogWarning(inException.what());
        setStatus(NodeCalculator::Status_Finished);
    }
    catch (const std::exception & inException)
    {
        LogError(inException.what());
        mErrorMessage = inException.what();
        setStatus(NodeCalculator::Status_Error);
    }
}


void NodeCalculatorImpl::start()
{
    mStatus.set(NodeCalculator::Status_Starting);
    mMainWorker.schedule(boost::bind(&NodeCalculatorImpl::startImpl, this));
}


} // namespace Tetris
