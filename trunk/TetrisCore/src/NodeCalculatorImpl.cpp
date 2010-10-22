#include "Tetris/NodeCalculatorImpl.h"
#include "Tetris/NodeCalculator.h"
#include "Tetris/AISupport.h"
#include "Tetris/GameQualityEvaluator.h"
#include "Tetris/GameStateComparisonFunctor.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/GameState.h"
#include "Tetris/BlockTypes.h"
#include "Tetris/WorkerPool.h"
#include "Tetris/Assert.h"
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
        mLayers(inBlockTypes.size()),
        mCompletedSearchDepth(0),
        mNode(inNode.release()),
        mBlockTypes(inBlockTypes),
        mWidths(inWidths),
        mEvaluator(inEvaluator.release()),
        mStatus(0),
        mMainWorker("NodeCalculatorImpl"),
        mWorkerPool(inWorkerPool)
    {
        Assert(!mNode->state().isGameOver());
        Assert(mNode->children().empty());
    }


    NodeCalculatorImpl::~NodeCalculatorImpl()
    {
    }


    void NodeCalculatorImpl::getLayerData(int inIndex, LayerData & outLayerData)
    {
        boost::mutex::scoped_lock lock(mLayersMutex);
        outLayerData = mLayers[inIndex];
    }


    int NodeCalculatorImpl::getCurrentSearchDepth() const
    {
        boost::mutex::scoped_lock lock(mCompletedSearchDepthMutex);
        return mCompletedSearchDepth;
    }


    void NodeCalculatorImpl::setCurrentSearchDepth(int inDepth)
    {
        boost::mutex::scoped_lock lock(mCompletedSearchDepthMutex);
        if (inDepth > mCompletedSearchDepth)
        {
            mCompletedSearchDepth = inDepth;
        }
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


    void NodeCalculatorImpl::updateLayerData(size_t inIndex, NodePtr inNodePtr, size_t inCount)
    {
        boost::mutex::scoped_lock lock(mLayersMutex);
        LayerData & layerData = mLayers[inIndex];
        layerData.mNumItems += inCount;
        if (!layerData.mBestChild || inNodePtr->state().quality(inNodePtr->evaluator()) > layerData.mBestChild->state().quality(inNodePtr->evaluator()))
        {
            layerData.mBestChild = inNodePtr;
        }
    }


    void NodeCalculatorImpl::populateNodesRecursively(
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
            updateLayerData(inIndex, *ioNode->children().begin(), count);
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


    void NodeCalculatorImpl::markTreeRowAsFinished(size_t inIndex)
    {
        setCurrentSearchDepth(inIndex + 1);
        boost::mutex::scoped_lock lock(mLayersMutex);
        mLayers[inIndex].mFinished = true;
    }


    void NodeCalculatorImpl::destroyInferiorChildren()
    {
        size_t reachedDepth = getCurrentSearchDepth();
        Assert(reachedDepth >= 1);

        // We use the 'best child' from this search depth.
        // The path between the start node and this best
        // child will be the list of precalculated nodes.
        boost::mutex::scoped_lock layersLock(mLayersMutex);
        boost::mutex::scoped_lock nodeLock(mNodeMutex);
        Assert((reachedDepth - 1) < mLayers.size());
        NodePtr endNode = mLayers[reachedDepth - 1].mBestChild;
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
