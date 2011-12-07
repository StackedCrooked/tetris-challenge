#include "Tetris/NodeCalculator.h"
#include "Tetris/NodeCalculatorImpl.h"
#include "Tetris/MultithreadedNodeCalculator.h"
#include "Tetris/SingleThreadedNodeCalculator.h"


namespace Tetris {


using namespace Futile;


static std::unique_ptr<NodeCalculatorImpl> CreateImpl(std::unique_ptr<GameStateNode> inNode,
                                                    const BlockTypes & inBlockTypes,
                                                    const std::vector<int> & inWidths,
                                                    const Evaluator & inEvaluator,
                                                    Worker & inMainWorker,
                                                    WorkerPool & inWorkerPool)
{
    if (inWorkerPool.size() > 1)
    {
        return std::unique_ptr<NodeCalculatorImpl>(
            new MultithreadedNodeCalculator(std::move(inNode), inBlockTypes, inWidths, inEvaluator, inMainWorker, inWorkerPool));
    }
    else if (inWorkerPool.size() == 1)
    {
        return std::unique_ptr<NodeCalculatorImpl>(
            new SingleThreadedNodeCalculator(std::move(inNode), inBlockTypes, inWidths, inEvaluator, inMainWorker, inWorkerPool));
    }
    else
    {
        throw std::logic_error("Failed to create NodeCalculator object because the given WorkerPool is empty.");
    }
}


NodeCalculator::NodeCalculator(std::unique_ptr<GameStateNode> inNode,
                               const BlockTypes & inBlockTypes,
                               const std::vector<int> & inWidths,
                               const Evaluator & inEvaluator,
                               Worker & inMainWorker,
                               WorkerPool & inWorkerPool) :
    mImpl(CreateImpl(std::move(inNode), inBlockTypes, inWidths, inEvaluator, inMainWorker, inWorkerPool).release())
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


std::size_t NodeCalculator::getCurrentNodeCount() const
{
    return mImpl->getCurrentNodeCount();
}


std::size_t NodeCalculator::getMaxNodeCount() const
{
    return mImpl->getMaxNodeCount();
}


int NodeCalculator::getCurrentSearchDepth() const
{
    return mImpl->getCurrentSearchDepth();
}


int NodeCalculator::getMaxSearchDepth() const
{
    return mImpl->getMaxSearchDepth();
}


NodePtr NodeCalculator::result() const
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
