#include "Tetris/Config.h"
#include "Tetris/NodeCalculator.h"
#include "Tetris/NodeCalculatorImpl.h"
#include "Tetris/MultithreadedNodeCalculator.h"
#include "Tetris/SingleThreadedNodeCalculator.h"


namespace Tetris
{
    
    static std::auto_ptr<NodeCalculatorImpl> CreateImpl(std::auto_ptr<GameStateNode> inNode,
                                                        const BlockTypes & inBlockTypes,
                                                        const std::vector<int> & inWidths,
                                                        std::auto_ptr<Evaluator> inEvaluator,
                                                        WorkerPool & inWorkerPool)
    {
        if (inWorkerPool.size() > 1)
        {
            return std::auto_ptr<NodeCalculatorImpl>(
                new MultithreadedNodeCalculator(inNode, inBlockTypes, inWidths, inEvaluator, inWorkerPool));
        }
        else if (inWorkerPool.size() == 1)
        {
            return std::auto_ptr<NodeCalculatorImpl>(
                new SingleThreadedNodeCalculator(inNode, inBlockTypes, inWidths, inEvaluator, inWorkerPool));
        }
        else
        {
            throw std::logic_error("Failed to create NodeCalculator object because the given WorkerPool is empty.");
        }
    }


    NodeCalculator::NodeCalculator(std::auto_ptr<GameStateNode> inNode,
                                   const BlockTypes & inBlockTypes,
                                   const std::vector<int> & inWidths,
                                   std::auto_ptr<Evaluator> inEvaluator,
                                   WorkerPool & inWorkerPool) :
        mImpl(CreateImpl(inNode, inBlockTypes, inWidths, inEvaluator, inWorkerPool).release())
    {
    }


    NodeCalculator::~NodeCalculator()
    {
        delete mImpl;
        mImpl = 0;
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


	const std::string & NodeCalculator::errorMessage() const
	{
		return mImpl->errorMessage();
	}

} // namespace Tetris
