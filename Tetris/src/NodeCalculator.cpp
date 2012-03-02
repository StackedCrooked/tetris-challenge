#include "Tetris/NodeCalculator.h"
#include "Tetris/NodeCalculatorImpl.h"
#include "Tetris/MultiThreadedNodeCalculator.h"
#include "Tetris/SingleThreadedNodeCalculator.h"
#include "Tetris/GameState.h"


namespace Tetris {


using namespace Futile;


static std::unique_ptr<NodeCalculatorImpl> CreateImpl(const GameState & inGameState,
                                                    const BlockTypes & inBlockTypes,
                                                    const std::vector<int> & inWidths,
                                                    const Evaluator & inEvaluator,
                                                    Worker & inMainWorker,
                                                    WorkerPool & inWorkerPool)
{
    if (inWorkerPool.size() > 1)
    {
        return std::unique_ptr<NodeCalculatorImpl>(
            new MultiThreadedNodeCalculator(inGameState,
                                            inBlockTypes,
                                            inWidths,
                                            inEvaluator,
                                            inMainWorker,
                                            inWorkerPool));
    }
    else if (inWorkerPool.size() == 1)
    {
        return std::unique_ptr<NodeCalculatorImpl>(
            new SingleThreadedNodeCalculator(inGameState,
                                             inBlockTypes,
                                             inWidths,
                                             inEvaluator,
                                             inMainWorker,
                                             inWorkerPool));
    }
    else
    {
        throw std::logic_error("Failed to create NodeCalculator object because the given WorkerPool is empty.");
    }
}


NodeCalculator::NodeCalculator(const GameState & inGameState,
                               const BlockTypes & inBlockTypes,
                               const std::vector<int> & inWidths,
                               const Evaluator & inEvaluator,
                               Worker & inMainWorker,
                               WorkerPool & inWorkerPool) :
    mImpl(CreateImpl(inGameState, inBlockTypes, inWidths, inEvaluator, inMainWorker, inWorkerPool).release())
{
}


NodeCalculator::~NodeCalculator()
{
    mImpl.reset();
}


void NodeCalculator::start()
{
    return mImpl->start();
}


void NodeCalculator::stop()
{
    mImpl->stop();
}


int NodeCalculator::getCurrentSearchDepth() const
{
    return mImpl->getCurrentSearchDepth();
}


int NodeCalculator::getMaxSearchDepth() const
{
    return mImpl->getMaxSearchDepth();
}


unsigned NodeCalculator::getCurrentNodeCount() const
{
    return mImpl->getCurrentNodeCount();
}


unsigned NodeCalculator::getMaxNodeCount() const
{
    return mImpl->getMaxNodeCount();
}


std::vector<GameState> NodeCalculator::getCurrentResults() const
{
    Assert(status() != Status_Error);
    return mImpl->result();
}


NodeCalculator::Status NodeCalculator::status() const
{
    return static_cast<Status>(mImpl->status());
}


std::string NodeCalculator::errorMessage() const
{
    return mImpl->errorMessage();
}


} // namespace Tetris
