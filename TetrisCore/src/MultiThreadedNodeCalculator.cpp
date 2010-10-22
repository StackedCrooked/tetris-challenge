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


    void MultithreadedNodeCalculator::populateNodesMt(
        NodePtr ioNode,
        const BlockTypes & inBlockTypes,
        const std::vector<int> & inWidths,
        size_t inIndex,
        size_t inMaxIndex)
    {

        //
        // Generate the child nodes.
        //
        // It is possible that the nodes were already generated at this depth.
        // If that is the case then we immediately jump to the recursive call below.
        //
        ChildNodes generatedChildNodes = ioNode->children();
        Assert(generatedChildNodes.empty());
        if (generatedChildNodes.empty())
        {
            generatedChildNodes = ChildNodes(GameStateComparisonFunctor(mEvaluator->clone()));
            GenerateOffspring(ioNode, inBlockTypes[inIndex], *mEvaluator, generatedChildNodes);

            int count = 0;
            ChildNodes::iterator it = generatedChildNodes.begin(), end = generatedChildNodes.end();
            while (count < inWidths[inIndex] && it != end)
            {
                ioNode->addChild(*it);
                ++count;
                ++it;
            }
            updateLayerData(inIndex, *ioNode->children().begin(), count);
        }

    }


    void MultithreadedNodeCalculator::populateNodes(
        NodePtr ioNode,
        const BlockTypes & inBlockTypes,
        const std::vector<int> & inWidths,
        size_t inIndex,
        size_t inMaxIndex)
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
        if (inIndex < inMaxIndex)
        {
            if (inIndex + 1 == inMaxIndex)
            {
                Assert(ioNode->children().empty());
                Worker::Task task = boost::bind(&MultithreadedNodeCalculator::populateNodesMt, this, ioNode, inBlockTypes, inWidths, inIndex, inMaxIndex);
                mWorkerPool.getWorker()->schedule(task);
            }
            else
            {            
                ChildNodes childNodes = ioNode->children();
                Assert(!childNodes.empty());
                for (ChildNodes::iterator it = childNodes.begin(); it != childNodes.end(); ++it)
                {
                    NodePtr child = *it;
                    populateNodes(child, inBlockTypes, inWidths, inIndex + 1, inMaxIndex);
                }                
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
