#include "Tetris/Config.h"
#include "Tetris/SingleThreadedNodeCalculator.h"
#include "Tetris/Evaluator.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/GameState.h"
#include "Tetris/BlockTypes.h"
#include "Tetris/WorkerPool.h"
#include "Tetris/Logging.h"
#include "Tetris/MakeString.h"
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <memory>
#include <stdexcept>


namespace Tetris
{

    SingleThreadedNodeCalculator::SingleThreadedNodeCalculator(std::auto_ptr<GameStateNode> inNode,
                                                               const BlockTypes & inBlockTypes,
                                                               const std::vector<int> & inWidths,
                                                               std::auto_ptr<Evaluator> inEvaluator,
                                                               WorkerPool & inWorkerPool) :
        NodeCalculatorImpl(inNode, inBlockTypes, inWidths, inEvaluator, inWorkerPool)
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
            size_t targetDepth = 1;
            while (targetDepth <= mBlockTypes.size())
            {
                boost::mutex::scoped_lock lock(mNodeMutex);
                populateNodesRecursively(mNode, mBlockTypes, mWidths, 0, targetDepth - 1);
                mTreeRowInfos.setFinished(targetDepth);
                targetDepth++;
            }
        }
        catch (const boost::thread_interrupted &)
        {
            // Task was interrupted. Ok.
        }
        catch (const std::exception & inException)
        {
            LogError(MakeString() << "Exception caught in SingleThreadedNodeCalculator::populate(). Detail: " << inException.what());
        }
        mWorkerPool.wait();
        Assert(getCurrentSearchDepth() >= 1 || getQuitFlag())
    }

} // namespace Tetris
