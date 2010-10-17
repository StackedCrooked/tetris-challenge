#include "Tetris/MultithreadedNodeCalculator.h"
#include "Tetris/GameQualityEvaluator.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/GameState.h"


namespace Tetris
{


    class MultithreadedNodeCalculatorImpl : public AbstractNodeCalculator
    {
    public:
        MultithreadedNodeCalculatorImpl(boost::shared_ptr<WorkerPool> inWorkerPool,
                                    std::auto_ptr<GameStateNode> inNode,
                                    const BlockTypes & inBlockTypes,
                                    const Widths & inWidths,
                                    std::auto_ptr<Evaluator> inEvaluator);

        virtual ~MultithreadedNodeCalculatorImpl();

        virtual void start();

        virtual void stop();

        virtual int getCurrentSearchDepth() const;

        virtual int getMaxSearchDepth() const;

        virtual NodePtr result() const;

        AbstractNodeCalculator::Status status() const;

    private:
        int mMaxSearchDepth;
        std::vector<boost::shared_ptr<AbstractNodeCalculator> > mNodeCalculators;
    };


    MultithreadedNodeCalculatorImpl::MultithreadedNodeCalculatorImpl(boost::shared_ptr<WorkerPool> inWorkerPool,
                                                                     std::auto_ptr<GameStateNode> inNode,
                                                                     const BlockTypes & inBlockTypes,
                                                                     const Widths & inWidths,
                                                                     std::auto_ptr<Evaluator> inEvaluator) :
        mMaxSearchDepth(inBlockTypes.size())
    {

    }


    MultithreadedNodeCalculatorImpl::~MultithreadedNodeCalculatorImpl()
    {
    }


    void MultithreadedNodeCalculatorImpl::start()
    {
        for (size_t idx = 0; idx != mNodeCalculators.size(); ++idx)
        {
            mNodeCalculators[idx]->start();
        }
    }


    void MultithreadedNodeCalculatorImpl::stop()
    {
        for (size_t idx = 0; idx != mNodeCalculators.size(); ++idx)
        {
            mNodeCalculators[idx]->stop();
        }
    }


    int MultithreadedNodeCalculatorImpl::getCurrentSearchDepth() const
    {
        int result = 0;
        for (size_t idx = 0; idx != mNodeCalculators.size(); ++idx)
        {
            if (idx == 0 || result < mNodeCalculators[idx]->getCurrentSearchDepth())
            {
                result = mNodeCalculators[idx]->getCurrentSearchDepth();
            }
        }
        return result;
    }


    int MultithreadedNodeCalculatorImpl::getMaxSearchDepth() const
    {
        return mMaxSearchDepth;
    }


    NodePtr MultithreadedNodeCalculatorImpl::result() const
    {
        NodePtr result;
        int reachedSearchDepth = 0;
        for (size_t idx = 0; idx != mNodeCalculators.size(); ++idx)
        {
            if (idx == 0 || reachedSearchDepth < mNodeCalculators[idx]->getCurrentSearchDepth())
            {
                result = mNodeCalculators[idx]->result();
                reachedSearchDepth = mNodeCalculators[idx]->getCurrentSearchDepth();
            }
        }
        return result;
    }


    AbstractNodeCalculator::Status MultithreadedNodeCalculatorImpl::status() const
    {
        AbstractNodeCalculator::Status result = AbstractNodeCalculator::Status_Nil;
        for (size_t idx = 0; idx != mNodeCalculators.size(); ++idx)
        {
            if (idx == 0 || result < mNodeCalculators[idx]->status())
            {
                result = mNodeCalculators[idx]->status();
            }
        }
        return result;
    }


    MultithreadedNodeCalculator::MultithreadedNodeCalculator(boost::shared_ptr<WorkerPool> inWorkerPool,
                                                             std::auto_ptr<GameStateNode> inNode,
                                                             const BlockTypes & inBlockTypes,
                                                             const Widths & inWidths,
                                                             std::auto_ptr<Evaluator> inEvaluator) :
        mImpl(new MultithreadedNodeCalculatorImpl(inWorkerPool, inNode, inBlockTypes, inWidths, inEvaluator))
    {

    }

    
    MultithreadedNodeCalculator::~MultithreadedNodeCalculator()
    {
        delete mImpl;
        mImpl = 0;
    }


    void MultithreadedNodeCalculator::start()
    {
        mImpl->start();
    }


    void MultithreadedNodeCalculator::stop()
    {
        mImpl->stop();
    }


    int MultithreadedNodeCalculator::getCurrentSearchDepth() const
    {
        return mImpl->getCurrentSearchDepth();
    }


    int MultithreadedNodeCalculator::getMaxSearchDepth() const
    {
        return mImpl->getMaxSearchDepth();
    }


    NodePtr MultithreadedNodeCalculator::result() const
    {
        return mImpl->result();
    }


    AbstractNodeCalculator::Status MultithreadedNodeCalculator::status() const
    {
        return mImpl->status();
    }

} // namespace Tetris
