#include "Tetris/SingleThreadedNodeCalculator.h"
#include "Tetris/AISupport.h"
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


namespace Tetris {


using namespace Futile;


SingleThreadedNodeCalculator::SingleThreadedNodeCalculator(const GameState & inGameState,
                                                           const BlockTypes & inBlockTypes,
                                                           const std::vector<int> & inWidths,
                                                           const Evaluator & inEvaluator,
                                                           Worker & inMainWorker,
                                                           WorkerPool & inWorkerPool) :
    NodeCalculatorImpl(inGameState, inBlockTypes, inWidths, inEvaluator, inMainWorker, inWorkerPool)
{
}


SingleThreadedNodeCalculator::~SingleThreadedNodeCalculator()
{
    setQuitFlag();
    mMainWorker.interruptAndClearQueue();
    mWorkerPool.interruptAndClearQueue();
}


void SingleThreadedNodeCalculator::onChildNodeGenerated(const Progress & , const NodePtr & inChildNode)
{
    stm::atomic([&](stm::transaction & tx) {
        mAllResults.registerNode(tx, inChildNode);
    });
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
            CalculateNodes(mRootNode,
                           mEvaluator,
                           mBlockTypes,
                           mWidths,
                           Progress(0, targetDepth),
                           boost::bind(&SingleThreadedNodeCalculator::onChildNodeGenerated, this, _1, _2));
            stm::atomic([&](stm::transaction & tx) {
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
    mWorkerPool.wait();
    Assert(getCurrentSearchDepth() >= 1 || getQuitFlag());
}

} // namespace Tetris
