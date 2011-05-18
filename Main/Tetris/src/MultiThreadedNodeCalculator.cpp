#include "Tetris/Config.h"
#include "Tetris/MultithreadedNodeCalculator.h"
#include "Tetris/AISupport.h"
#include "Tetris/GameStateComparator.h"
#include "Tetris/Evaluator.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/GameState.h"
#include "Tetris/BlockTypes.h"
#include "Futile/WorkerPool.h"
#include "Futile/Logging.h"
#include "Futile/Assert.h"
#include "Futile/MakeString.h"
#include "Futile/Threading.h"
#include <boost/shared_ptr.hpp>
#include <memory>
#include <stdexcept>


using Futile::LogWarning;
using Futile::ScopedLock;
using Futile::Worker;
using Futile::WorkerPool;


namespace Tetris {


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
    setQuitFlag();
    mMainWorker.interruptAndClearQueue();
    mWorkerPool.interruptAndClearQueue();
}


void MultithreadedNodeCalculator::generateChildNodes(NodePtr ioNode,
                                                     boost::shared_ptr<Evaluator> inEvaluator,
                                                     BlockType inBlockType,
                                                     int inDepth,
                                                     int inWidth)
{
    ChildNodes childNodes = ChildNodes(GameStateComparator(inEvaluator->clone()));
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


std::size_t MultithreadedNodeCalculator::populateNodes(NodePtr ioNode,
													   const BlockTypes & inBlockTypes,
													   const std::vector<int> & inWidths,
													   std::size_t inIndex,
													   std::size_t inEndIndex)
{
	std::size_t result = inIndex;

    // We want to at least perform a search of depth 1.
    if (inIndex > 0)
    {		
        boost::this_thread::interruption_point();
    }


    if (ioNode->gameState().isGameOver())
    {
        // GameOver state has no children.
        return result;
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
            result = populateNodes(child, inBlockTypes, inWidths, inIndex + 1, inEndIndex);
        }
    }
	return result;
}


void MultithreadedNodeCalculator::populate()
{
    try
    {
        // The nodes are populated using a "Iterative deepening" algorithm.
        // See: http://en.wikipedia.org/wiki/Iterative_deepening_depth-first_search for more information.
        std::size_t targetDepth = 1;
        while (targetDepth <= mBlockTypes.size())
        {
            ScopedLock lock(mNodeMutex);
			std::size_t reachedDepth = populateNodes(mNode, mBlockTypes, mWidths, 0, targetDepth);
			Assert(reachedDepth + 1 == targetDepth);
            mWorkerPool.wait();
            Assert(mWorkerPool.getActiveWorkerCount() == 0);
            mTreeRowInfos.setFinished(targetDepth);
            targetDepth++;
        }
    }
    catch (const boost::thread_interrupted &)
    {
        // Task was interrupted. Ok.
    }
    //
    // catch: allow other exceptions pass to the parent handler
    //

    mWorkerPool.interruptAndClearQueue();
    mWorkerPool.wait();
}

} // namespace Tetris