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
    stm::atomic([&](stm::transaction & tx) {
        mImpl->start(tx);
    });
}


void NodeCalculator::stop()
{
    stm::atomic([&](stm::transaction & tx) {
        mImpl->stop(tx);
    });
}


int NodeCalculator::getCurrentSearchDepth() const
{
    return stm::atomic<int>([&](stm::transaction & tx) {
        return mImpl->getCurrentSearchDepth(tx);
    });
}


int NodeCalculator::getMaxSearchDepth() const
{
    return mImpl->getMaxSearchDepth();
}


std::vector<GameState> NodeCalculator::result() const
{
    return stm::atomic< std::vector<GameState> >([&](stm::transaction & tx) {
        Assert(mImpl->status(tx) != Status_Error);
        return mImpl->result(tx);
    });
}


NodeCalculator::Status NodeCalculator::status() const
{
    return stm::atomic<NodeCalculator::Status>([&](stm::transaction & tx) {
        return static_cast<Status>(mImpl->status(tx));
    });
}


std::string NodeCalculator::errorMessage() const
{
    return stm::atomic<std::string>([&](stm::transaction & tx) {
        return mImpl->errorMessage(tx);
    });
}


} // namespace Tetris
