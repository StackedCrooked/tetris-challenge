#include "Tetris/Config.h"
#include "Tetris/MultithreadedNodeCalculator.h"
#include "Tetris/AISupport.h"
#include "Tetris/GameStateComparisonFunctor.h"
#include "Tetris/GameQualityEvaluator.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/GameState.h"
#include "Tetris/BlockTypes.h"
#include "Tetris/WorkerPool.h"
#include "Tetris/Logging.h"
#include "Tetris/Assert.h"
#include "Tetris/MakeString.h"
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <memory>
#include <stdexcept>


namespace Tetris
{

    MultithreadedNodeCalculator::MultithreadedNodeCalculator(std::auto_ptr<GameStateNode> inNode,
                                                             const BlockTypes & inBlockTypes,
                                                             const std::vector<int> & inWidths,
                                                             std::auto_ptr<Evaluator> inEvaluator,
                                                             WorkerPool & inWorkerPool) :
        NodeCalculatorImpl(inNode, inBlockTypes, inWidths, inEvaluator, inWorkerPool)
    {
    }
		
		
	MultithreadedNodeCalculator::~MultithreadedNodeCalculator()
	{
        mMainWorker.interruptAndClearQueue();
		mWorkerPool.interruptAndClearQueue();
	}


    void MultithreadedNodeCalculator::generateChildNodes(NodePtr ioNode,
                                                         boost::shared_ptr<Evaluator> inEvaluator,
														 BlockType inBlockType,
														 int inDepth,
														 int inWidth)
    {
        ChildNodes childNodes = ChildNodes(GameStateComparisonFunctor(inEvaluator->clone()));
        GenerateOffspring(ioNode, inBlockType, *inEvaluator, childNodes);
        if (childNodes.empty())
        {
            throw std::logic_error("GenerateOffspring produced zero children. This should not happen!");
        }

        int count = 0;
        ChildNodes::iterator it = childNodes.begin(), end = childNodes.end();
        while (count < inWidth && it != end)
        {
            ioNode->addChild(*it);
            ++count;
            ++it;
        }
        mTreeRowInfos.registerNode(*ioNode->children().begin(), inDepth);
    }


    void MultithreadedNodeCalculator::populateNodes(NodePtr ioNode,
		                                            const BlockTypes & inBlockTypes,
													const std::vector<int> & inWidths,
													size_t inIndex,
													size_t inEndIndex)
    {

        // We want to at least perform a depth - 1 search.
        if (inIndex > 0)
        {
            boost::this_thread::interruption_point();
        }


        if (ioNode->state().isGameOver())
        {
            // Game over state has no children.
            return;
        }

        //
        // Check stop conditions
        //
        Assert(inIndex < inEndIndex);
        if (inIndex + 1 == inEndIndex)
        {
            Assert(ioNode->children().empty());
            boost::shared_ptr<Evaluator> evaluator(mEvaluator->clone().release());
			Worker::Task task = boost::bind(&MultithreadedNodeCalculator::generateChildNodes,
				                            this,
											ioNode,
										    evaluator,
											inBlockTypes[inIndex],
											inIndex + 1,
											inWidths[inIndex]);
            mWorkerPool.schedule(task);
            
            // End of recursion.
        }
        else
        {            
            ChildNodes childNodes = ioNode->children();
            if (childNodes.empty())
            {
                LogWarning("Nodes have disappeared.");
            }

            for (ChildNodes::iterator it = childNodes.begin(); it != childNodes.end(); ++it)
            {
                NodePtr child = *it;

                // Start recursion.
                populateNodes(child, inBlockTypes, inWidths, inIndex + 1, inEndIndex);
            }                
        }
    }


    void MultithreadedNodeCalculator::populate()
    {
        try
        {
            // The nodes are populated using a "Iterative deepening" algorithm.
            // See: http://en.wikipedia.org/wiki/Iterative_deepening_depth-first_search for more information.
            size_t targetDepth = 1;
            while (targetDepth <= mBlockTypes.size())
            {
                boost::mutex::scoped_lock lock(mNodeMutex);
                populateNodes(mNode, mBlockTypes, mWidths, 0, targetDepth);
                mWorkerPool.wait();
                Assert(mWorkerPool.getActiveWorkerCount() == 0);
                mTreeRowInfos.setFinished(targetDepth);
                targetDepth++;
            }
        }
        catch (const boost::thread_interrupted &)
        {
            // Task was interrupted. Ok.
            if (getCurrentSearchDepth() >= 1)
            {
                mWorkerPool.interruptAndClearQueue();
            }
        }
        catch (const std::exception & inException)
        {
            LogError(MakeString() << "Exception caught in MultithreadedNodeCalculator::populate(). Detail: " << inException.what());
            mWorkerPool.interruptAndClearQueue();
        }        
        mWorkerPool.wait();
        Assert(getCurrentSearchDepth() >= 1);
    }

} // namespace Tetris
