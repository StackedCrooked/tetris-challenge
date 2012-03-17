#include "Tetris/MultiThreadedNodeCalculator.h"
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
#include <boost/shared_ptr.hpp>
#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>


namespace Tetris {


using namespace Futile;


MultiThreadedNodeCalculator::MultiThreadedNodeCalculator(const GameState & inGameState,
                                                         const BlockTypes & inBlockTypes,
                                                         const std::vector<int> & inWidths,
                                                         const Evaluator & inEvaluator,
                                                         Futile::Worker & inMainWorker,
                                                         WorkerPool & inWorkerPool) :
    NodeCalculatorImpl(inGameState, inBlockTypes, inWidths, inEvaluator, inMainWorker, inWorkerPool)
{
}


MultiThreadedNodeCalculator::~MultiThreadedNodeCalculator()
{
    setQuitFlag();
    mMainWorker.interruptAndClearQueue();
    mWorkerPool.interruptAndClearQueue();
}


void MultiThreadedNodeCalculator::generateChildNodes(const NodePtr & ioNode,
                                                     const Evaluator * inEvaluator,
                                                     BlockType inBlockType,
                                                     int inMaxChildCount)
{
    ChildNodes childNodes;
    GenerateOffspring(ioNode, inBlockType, *inEvaluator, childNodes);
    if (childNodes.empty())
    {
        throw std::logic_error("GenerateOffspring produced zero children. This should not happen!");
    }

    int count = 0;
    ChildNodes::iterator it = childNodes.begin(), end = childNodes.end();
    while (count < inMaxChildCount && it != end)
    {
        ioNode->addChild(*it);
        ++count;
        ++it;
    }


    stm::atomic([&](stm::transaction & tx) {
        mAllResults.registerNode(tx, *ioNode->children().begin());
    });
}


void MultiThreadedNodeCalculator::populateNodes(const NodePtr & ioNode,
                                                const BlockTypes & inBlockTypes,
                                                const std::vector<int> & inWidths,
                                                std::size_t inIndex,
                                                std::size_t inEndIndex)
{

    // We want to at least perform a search of depth 4.
    if (inIndex >= 4)
    {
        boost::this_thread::interruption_point();
    }


    if (ioNode->gameState().isGameOver())
    {
        // GameOver state has no children.
        return;
    }

    //
    // Check stop conditions
    //
    Assert(inIndex < inEndIndex);
    if (inIndex + 1 == inEndIndex)
    {
        Assert(ioNode->children().empty());
        const int cWidth = inWidths[inIndex];
        if (cWidth == 0)
        {
            throw std::logic_error("Width is zero.");
        }

        const BlockType cBlockType = inBlockTypes[inIndex];
        mWorkerPool.schedule([=]() {
            generateChildNodes(ioNode, &mEvaluator, cBlockType, cWidth);
        });

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
            NodePtr childNode = *it;
            populateNodes(childNode, inBlockTypes, inWidths, inIndex + 1, inEndIndex);
        }
    }
}


void MultiThreadedNodeCalculator::populate()
{
    try
    {
        // The nodes are populated using a "Iterative deepening" algorithm.
        // See: http://en.wikipedia.org/wiki/Iterative_deepening_depth-first_search for more information.
        std::size_t targetDepth = 1;
        while (targetDepth <= mBlockTypes.size())
        {
            populateNodes(mNode, mBlockTypes, mWidths, 0, targetDepth);
            mWorkerPool.wait();
            Assert(mWorkerPool.getActiveWorkerCount() == 0);
            if (mNode->endNode()->gameState().isGameOver())
            {
                break;
            }

            stm::atomic([&](stm::transaction & tx)
            {
                mAllResults.setFinished(tx);
                calculateResult(tx);
            });

            targetDepth++;
        }
    }
    catch (const boost::thread_interrupted &)
    {
        // Task was interrupted. Ok.
    }
    // don't catch other exceptions here

    mWorkerPool.interruptAndClearQueue();
    mWorkerPool.wait();
}


} // namespace Tetris
