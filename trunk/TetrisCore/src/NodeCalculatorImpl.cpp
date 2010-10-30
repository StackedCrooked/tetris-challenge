#include "Tetris/Config.h"
#include "Tetris/NodeCalculatorImpl.h"
#include "Tetris/NodeCalculator.h"
#include "Tetris/AISupport.h"
#include "Tetris/GameQualityEvaluator.h"
#include "Tetris/GameStateComparisonFunctor.h"
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

    NodeCalculatorImpl::NodeCalculatorImpl(std::auto_ptr<GameStateNode> inNode,
                                           const BlockTypes & inBlockTypes,
                                           const std::vector<int> & inWidths,
                                           std::auto_ptr<Evaluator> inEvaluator,
                                           WorkerPool & inWorkerPool) :
        mNode(inNode.release()),
        mNodeMutex(),
        mQuitFlag(false),
        mQuitFlagMutex(),
        mTreeRowInfos(inEvaluator->clone(), inBlockTypes.size()),
        mBlockTypes(inBlockTypes),
        mWidths(inWidths),
        mEvaluator(inEvaluator->clone()),
        mStatus(0),
        mStatusMutex(),
        mMainWorker("NodeCalculatorImpl"),
        mWorkerPool(inWorkerPool)
    {
        Assert(!mNode->state().isGameOver());
        Assert(mNode->children().empty());
    }


    NodeCalculatorImpl::~NodeCalculatorImpl()
    {
    }


    void NodeCalculatorImpl::setQuitFlag()
    {
        boost::mutex::scoped_lock lock(mQuitFlagMutex);
        mQuitFlag = true;
    }


    bool NodeCalculatorImpl::getQuitFlag() const
    {
        boost::mutex::scoped_lock lock(mQuitFlagMutex);
        return mQuitFlag;
    }


    int NodeCalculatorImpl::getCurrentSearchDepth() const
    {
        return mTreeRowInfos.currentSearchDepth();
    }


    int NodeCalculatorImpl::getMaxSearchDepth() const
    {
        return mWidths.size();
    }


    NodePtr NodeCalculatorImpl::result() const
    {
        Assert(status() == NodeCalculator::Status_Finished);

        boost::mutex::scoped_lock lock(mNodeMutex);
        
        // Impossible since the thread interruption point is
        // not triggered before search depth 1 is complete.
        Assert(!mNode->children().empty());

        size_t numChildren = mNode->children().size();

        // DestroyInferiorChildren should
        // have taken care of this.
        Assert(numChildren == 1);

        return *mNode->children().begin();
    }


    int NodeCalculatorImpl::status() const
    {
        boost::mutex::scoped_lock lock(mStatusMutex);
        return mStatus;
    }


    void NodeCalculatorImpl::setStatus(int inStatus)
    {
        boost::mutex::scoped_lock lock(mStatusMutex);
        mStatus = inStatus;
    }


    void NodeCalculatorImpl::populateNodesRecursively(
        NodePtr ioNode,
        const BlockTypes & inBlockTypes,
        const std::vector<int> & inWidths,
        size_t inIndex,
        size_t inMaxIndex)
    {

        // We want to at least perform a search of depth 1.
        if (inIndex > 0)
        {
            boost::this_thread::interruption_point();
        }

        //
        // Check stop conditions
        //
        if (inIndex > inMaxIndex || inIndex >= inBlockTypes.size())
        {
            return;
        }


        if (ioNode->state().isGameOver())
        {
            // Game over state has no children.
            return;
        }


        //
        // Generate the child nodes.
        //
        // It is possible that the nodes were already generated at this depth.
        // If that is the case then we immediately jump to the recursive call below.
        //
        ChildNodes generatedChildNodes = ioNode->children();
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
            Assert(count >= 1);
            mTreeRowInfos.registerNode(*ioNode->children().begin(), inIndex + 1);
        }


        //
        // Recursive call on each child node.
        //
        if (inIndex < inMaxIndex)
        {
            for (ChildNodes::iterator it = generatedChildNodes.begin(); it != generatedChildNodes.end(); ++it)
            {
                NodePtr child = *it;
                populateNodesRecursively(child, inBlockTypes, inWidths, inIndex + 1, inMaxIndex);
            }
        }
    }


    void NodeCalculatorImpl::destroyInferiorChildren()
    {
        size_t reachedDepth = getCurrentSearchDepth();
        if (reachedDepth == 0)
        {
            Assert(getQuitFlag());
            return;
        }

        // We use the 'best child' from this search depth.
        // The path between the start node and this best
        // child will be the list of precalculated nodes.
        boost::mutex::scoped_lock nodeLock(mNodeMutex);
        NodePtr endNode = mTreeRowInfos.bestNode();
        Assert(endNode);
        if (endNode)
        {
            CarveBestPath(mNode, endNode);
            Assert(mNode->children().size() == 1);
        }
    }


    void NodeCalculatorImpl::stop()
    {
        if (status() == NodeCalculator::Status_Started || status() == NodeCalculator::Status_Working)
        {
            setStatus(NodeCalculator::Status_Stopped);
            mMainWorker.interrupt();
            Assert(mMainWorker.size() == 0);
            mWorkerPool.interruptAndClearQueue();
        }
    }


    void NodeCalculatorImpl::startImpl()
    {
        // Thread entry point has try/catch block
        try
        {
            setStatus(NodeCalculator::Status_Working);
            populate();
            destroyInferiorChildren();
        }
        catch (const std::exception & inException)
        {
            LogError(MakeString() << inException.what());
        }
        setStatus(NodeCalculator::Status_Finished);
    }


    void NodeCalculatorImpl::start()
    {
        boost::mutex::scoped_lock lock(mStatusMutex);
        Assert(mStatus == NodeCalculator::Status_Nil);
        mMainWorker.schedule(boost::bind(&NodeCalculatorImpl::startImpl, this));
        mStatus = NodeCalculator::Status_Started;
    }

} // namespace Tetris
