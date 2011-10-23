#include "Tetris/Config.h"
#include "Tetris/NodeCalculator.h"
#include "Tetris/NodeCalculatorImpl.h"
#include "Tetris/MultithreadedNodeCalculator.h"
#include "Tetris/SingleThreadedNodeCalculator.h"


namespace Tetris {


using namespace Futile;


static std::auto_ptr<NodeCalculatorImpl> CreateImpl(std::auto_ptr<GameStateNode> inNode,
                                                    const BlockTypes & inBlockTypes,
                                                    const std::vector<int> & inWidths,
                                                    const Evaluator & inEvaluator,
                                                    Worker & inMainWorker,
                                                    WorkerPool & inWorkerPool)
{
    if (inWorkerPool.size() > 1)
    {
        return std::auto_ptr<NodeCalculatorImpl>(
            new MultithreadedNodeCalculator(inNode, inBlockTypes, inWidths, inEvaluator, inMainWorker, inWorkerPool));
    }
    else if (inWorkerPool.size() == 1)
    {
        return std::auto_ptr<NodeCalculatorImpl>(
            new SingleThreadedNodeCalculator(inNode, inBlockTypes, inWidths, inEvaluator, inMainWorker, inWorkerPool));
    }
    else
    {
        throw std::logic_error("Failed to create NodeCalculator object because the given WorkerPool is empty.");
    }
}


NodeCalculator::NodeCalculator(std::auto_ptr<GameStateNode> inNode,
                               const BlockTypes & inBlockTypes,
                               const std::vector<int> & inWidths,
                               const Evaluator & inEvaluator,
                               Worker & inMainWorker,
                               WorkerPool & inWorkerPool) :
    mImpl(CreateImpl(inNode, inBlockTypes, inWidths, inEvaluator, inMainWorker, inWorkerPool).release())
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
