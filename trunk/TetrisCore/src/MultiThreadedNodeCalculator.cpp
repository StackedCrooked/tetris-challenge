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
		// We must stop all workers here or else they will be referencing a destroyed object.
		mWorkerPool.interruptAndClearQueue();
	}


    void MultithreadedNodeCalculator::generateChildNodes(NodePtr ioNode,
		                                                 Evaluator * inEvaluator,
														 BlockType inBlockType,
														 int inDepth,
														 int inWidth)
    {
		std::auto_ptr<Evaluator> evaluatorPtr(inEvaluator);
        //
        // Generate the child nodes.
        //
        // It is possible that the nodes were already generated at this depth.
        // If that is the case then we immediately jump to the recursive call below.
        //
        ChildNodes childNodes = ioNode->children();
		const Evaluator & evaluator(*evaluatorPtr);
		childNodes = ChildNodes(GameStateComparisonFunctor(evaluatorPtr));
        GenerateOffspring(ioNode, inBlockType, evaluator, childNodes);

        int count = 0;
        ChildNodes::iterator it = childNodes.begin(), end = childNodes.end();
        while (count < inWidth && it != end)
        {
            ioNode->addChild(*it);
            ++count;
            ++it;
        }
		updateLayerData(inDepth - 1, *ioNode->children().begin(), count);
    }


    void MultithreadedNodeCalculator::populateNodes(NodePtr ioNode,
		                                            const BlockTypes & inBlockTypes,
													const std::vector<int> & inWidths,
													size_t inIndex,
													size_t inEndIndex)
    {

        // We want to at least perform a depth-1 search.
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
			Worker::Task task = boost::bind(&MultithreadedNodeCalculator::generateChildNodes,
				                            this,
											ioNode,
											mEvaluator->clone().release(),
											inBlockTypes[inIndex],
											inIndex + 1,
											inWidths[inIndex]);
            mWorkerPool.getWorker()->schedule(task);
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
                populateNodes(child, inBlockTypes, inWidths, inIndex + 1, inEndIndex);
            }                
        }
    }


    void MultithreadedNodeCalculator::populate()
    {

        //
        // Fallback to single threaded implementation.
        //
        // Todo: implement
        // 
        try
        {
            // The nodes are populated using a "Iterative deepening" algorithm.
            // See: http://en.wikipedia.org/wiki/Iterative_deepening_depth-first_search for more information.
            size_t targetDepth = 1;
            while (targetDepth <= mBlockTypes.size())
            {
                boost::mutex::scoped_lock lock(mNodeMutex);
                populateNodes(mNode, mBlockTypes, mWidths, 0, targetDepth);
                mWorkerPool.waitForAll();
                markTreeRowAsFinished(targetDepth - 1);
                targetDepth++;
            }
        }
        catch (const boost::thread_interrupted &)
        {
            // Task was interrupted. Ok.
            mWorkerPool.interruptAndClearQueue();
        }
        catch (const std::exception & inException)
        {
            LogError(MakeString() << "Exception caught in MultithreadedNodeCalculator::populate(). Detail: " << inException.what());
        }
    }

} // namespace Tetris
