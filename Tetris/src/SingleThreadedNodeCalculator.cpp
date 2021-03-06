#include "Tetris/Config.h"
#include "Tetris/SingleThreadedNodeCalculator.h"
#include "Tetris/Evaluator.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/GameState.h"
#include "Tetris/BlockTypes.h"
#include "Futile/WorkerPool.h"
#include "Futile/Logging.h"
#include "Futile/MakeString.h"
#include "Futile/Threading.h"
#include <boost/shared_ptr.hpp>
#include <memory>
#include <stdexcept>


using Futile::ScopedLock;
using Futile::WorkerPool;


namespace Tetris {


SingleThreadedNodeCalculator::SingleThreadedNodeCalculator(std::unique_ptr<GameStateNode> inNode,
                                                           const BlockTypes& inBlockTypes,
                                                           const std::vector<int>& inWidths,
                                                           const Evaluator& inEvaluator,
                                                           WorkerPool& inWorkerPool) :
    NodeCalculatorImpl(std::move(inNode), inBlockTypes, inWidths, inEvaluator, inWorkerPool)
{
}


SingleThreadedNodeCalculator::~SingleThreadedNodeCalculator()
{
    setQuitFlag();
    mMainWorker.interruptAndClearQueue();
    mWorkerPool.interruptAndClearQueue();
}


void SingleThreadedNodeCalculator::populate()
{
    try
    {
        // The nodes are populated using a "Iterative deepening" algorithm.
        // See: http://en.wikipedia.org/wiki/Iterative_deepening_depth-first_search for more information.
        std::size_t targetDepth = 1;
        while (targetDepth <= mBlockTypes.size())
        {
            ScopedLock lock(mNodeMutex);
            populateNodesRecursively(mNode, mBlockTypes, mWidths, 0, targetDepth - 1);
            mTreeRowInfos.setFinished();
            targetDepth++;
        }
    }
    catch (const boost::thread_interrupted &)
    {
        // Task was interrupted. Ok.
    }
    mWorkerPool.wait();
    Assert(getCurrentSearchDepth() >= 1 || getQuitFlag());
}

} // namespace Tetris
