#include "Tetris/Config.h"
#include "Tetris/NodeCalculator.h"
#include "Tetris/NodeCalculatorImpl.h"
#include "Tetris/MultithreadedNodeCalculator.h"
#include "Tetris/SingleThreadedNodeCalculator.h"


using Futile::WorkerPool;


namespace Tetris {


static std::unique_ptr<NodeCalculatorImpl> CreateImpl(std::unique_ptr<GameStateNode> inNode,
                                                    const BlockTypes & inBlockTypes,
                                                    const std::vector<int> & inWidths,
                                                    const Evaluator & inEvaluator,
                                                    WorkerPool & inWorkerPool)
{
    if (inWorkerPool.size() > 1)
    {
        return std::unique_ptr<NodeCalculatorImpl>(
            new MultithreadedNodeCalculator(std::move(inNode), inBlockTypes, inWidths, inEvaluator, inWorkerPool));
    }
    else if (inWorkerPool.size() == 1)
    {
        return std::unique_ptr<NodeCalculatorImpl>(
            new SingleThreadedNodeCalculator(std::move(inNode), inBlockTypes, inWidths, inEvaluator, inWorkerPool));
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
                               WorkerPool & inWorkerPool) :
    mImpl(CreateImpl(std::move(inNode), inBlockTypes, inWidths, inEvaluator, inWorkerPool).release())
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
