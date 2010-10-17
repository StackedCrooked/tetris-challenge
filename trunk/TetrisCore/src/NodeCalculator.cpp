#include "Tetris/NodeCalculator.h"
#include "Tetris/AISupport.h"
#include "Tetris/GameStateComparisonFunctor.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/GameQualityEvaluator.h"
#include "Tetris/GameState.h"
#include "Tetris/Worker.h"
#include "Tetris/Logging.h"
#include "Tetris/MakeString.h"
#include "Tetris/Assert.h"
#include <boost/scoped_ptr.hpp>
#include <boost/thread/mutex.hpp>


namespace Tetris
{

    class NodeCalculatorImpl
    {
    public:
        NodeCalculatorImpl(boost::shared_ptr<Worker> inWorker,
                           std::auto_ptr<GameStateNode> inNode,
                           const BlockTypes & inBlockTypes,
                           const Widths & inWidths,
                           std::auto_ptr<Evaluator> inEvaluator);

        ~NodeCalculatorImpl();

        void start();

        void stop();

        int getCurrentSearchDepth() const;

        int getMaxSearchDepth() const;

        NodePtr result() const;

        AbstractNodeCalculator::Status status() const;

        // LayerData contains the accumulated data for all branches at a same depth.
        struct LayerData
        {
            LayerData() :
                mBestChild(),
                mNumItems(0),
                mFinished(false)
            {
            }

            NodePtr mBestChild;
            int mNumItems;
            bool mFinished;
        };

        void getLayerData(int inIndex, LayerData & outLayerData);

    private:
        void startImpl();

        void setStatus(AbstractNodeCalculator::Status inStatus);

        void setCurrentSearchDepth(int inDepth);

        void updateLayerData(size_t inIndex, NodePtr inNodePtr, size_t inCount);

        void markTreeRowAsFinished(size_t inIndex);
        void populate();
        void destroyInferiorChildren();

        void populateNodesRecursively(NodePtr ioNode,
                                      const BlockTypes & inBlockTypes,
                                      const Widths & inWidths,
                                      size_t inIndex,
                                      size_t inMaxIndex);

        NodePtr mNode;
        mutable boost::mutex mNodeMutex;

        // Store info per horizontal level of nodes.
        std::vector<LayerData> mLayers;
        mutable boost::mutex mLayersMutex;

        int mCompletedSearchDepth;
        mutable boost::mutex mCompletedSearchDepthMutex;

        BlockTypes mBlockTypes;
        Widths mWidths;
        boost::scoped_ptr<Evaluator> mEvaluator;

        AbstractNodeCalculator::Status mStatus;
        mutable boost::mutex mStatusMutex;

        boost::shared_ptr<Worker> mWorker;
        bool mDestroyedInferiorChildren; // just for testing
    };


    NodeCalculatorImpl::NodeCalculatorImpl(boost::shared_ptr<Worker> inWorker,
                                           std::auto_ptr<GameStateNode> inNode,
                                           const BlockTypes & inBlockTypes,
                                           const Widths & inWidths,
                                           std::auto_ptr<Evaluator> inEvaluator) :
        mLayers(inBlockTypes.size()),
        mCompletedSearchDepth(0),
        mNode(inNode.release()),
        mBlockTypes(inBlockTypes),
        mWidths(inWidths),
        mEvaluator(inEvaluator.release()),
        mStatus(AbstractNodeCalculator::Status_Nil),
        mWorker(inWorker),
        mDestroyedInferiorChildren(false)
    {
        Assert(!mNode->state().isGameOver());
        Assert(mNode->children().empty());
    }


    NodeCalculatorImpl::~NodeCalculatorImpl()
    {
        mWorker->interruptAndClearQueue();
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
        Assert(status() == AbstractNodeCalculator::Status_Finished);

        boost::mutex::scoped_lock lock(mNodeMutex);
        
        // Impossible since the thread interruption point is
        // not triggered before search depth 1 is complete.
        Assert(!mNode->children().empty());

        size_t numChildren = mNode->children().size();

        // DestroyInferiorChildren should
        // have taken care of this.
        Assert(mDestroyedInferiorChildren);
        Assert(numChildren == 1);

        return *mNode->children().begin();
    }


    AbstractNodeCalculator::Status NodeCalculatorImpl::status() const
    {
        boost::mutex::scoped_lock lock(mStatusMutex);
        return mStatus;
    }


    void NodeCalculatorImpl::setStatus(AbstractNodeCalculator::Status inStatus)
    {
        boost::mutex::scoped_lock lock(mStatusMutex);
        mStatus = inStatus;
    }


    void NodeCalculatorImpl::updateLayerData(size_t inIndex, NodePtr inNodePtr, size_t inCount)
    {
        boost::mutex::scoped_lock lock(mLayersMutex);
        LayerData & layerData = mLayers[inIndex];
        layerData.mNumItems += inCount;
        if (!layerData.mBestChild || inNodePtr->state().quality(inNodePtr->qualityEvaluator()) > layerData.mBestChild->state().quality(inNodePtr->qualityEvaluator()))
        {
            layerData.mBestChild = inNodePtr;
        }
    }


    void NodeCalculatorImpl::populateNodesRecursively(
        NodePtr ioNode,
        const BlockTypes & inBlockTypes,
        const Widths & inWidths,
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


    void NodeCalculatorImpl::populate()
    {
        try
        {
            // The nodes are populated using a "Iterative deepening" algorithm.
            // See: http://en.wikipedia.org/wiki/Iterative_deepening_depth-first_search for more information.
            size_t targetDepth = 0;
            while (targetDepth < mBlockTypes.size())
            {
                boost::mutex::scoped_lock lock(mNodeMutex);
                populateNodesRecursively(mNode, mBlockTypes, mWidths, 0, targetDepth);
                markTreeRowAsFinished(targetDepth);
                targetDepth++;
            }
        }
        catch (const boost::thread_interrupted &)
        {
            // Task was interrupted. Ok.
        }
        catch (const std::exception & inException)
        {
            LogError(MakeString() << "Exception caught in NodeCalculatorImpl::populate(). Detail: " << inException.what());
        }
    }


    void NodeCalculatorImpl::destroyInferiorChildren()
    {
        Assert(!mDestroyedInferiorChildren);
        size_t reachedDepth = getCurrentSearchDepth();
        Assert(reachedDepth >= 1);

        // We use the 'best child' from this search depth.
        // The path between the start node and this best
        // child will be the list of precalculated nodes.
        boost::mutex::scoped_lock layersLock(mLayersMutex);
        boost::mutex::scoped_lock nodeLock(mNodeMutex);
        Assert((reachedDepth - 1) < mLayers.size());
        CarveBestPath(mNode, mLayers[reachedDepth - 1].mBestChild);
        Assert(mNode->children().size() == 1);
        mDestroyedInferiorChildren = true;
    }


    void NodeCalculatorImpl::stop()
    {
        if (status() == AbstractNodeCalculator::Status_Started || status() == AbstractNodeCalculator::Status_Working)
        {
            setStatus(AbstractNodeCalculator::Status_Stopped);
            mWorker->interruptAndClearQueue();
        }
    }


    void NodeCalculatorImpl::startImpl()
    {
        // Thread entry point has try/catch block
        try
        {
            setStatus(AbstractNodeCalculator::Status_Working);
            populate();
            destroyInferiorChildren();
        }
        catch (const std::exception & inException)
        {
            LogError(MakeString() << inException.what());
        }
        setStatus(AbstractNodeCalculator::Status_Finished);
    }


    void NodeCalculatorImpl::start()
    {
        boost::mutex::scoped_lock lock(mStatusMutex);
        Assert(mStatus == AbstractNodeCalculator::Status_Nil);
        mWorker->schedule(boost::bind(&NodeCalculatorImpl::startImpl, this));
        mStatus = AbstractNodeCalculator::Status_Started;
    }

    
    NodeCalculator::NodeCalculator(boost::shared_ptr<Worker> inWorker,
                                   std::auto_ptr<GameStateNode> inNode,
                                   const BlockTypes & inBlockTypes,
                                   const Widths & inWidths,
                                   std::auto_ptr<Evaluator> inEvaluator) :
        AbstractNodeCalculator(),
        mImpl(new NodeCalculatorImpl(inWorker, inNode, inBlockTypes, inWidths, inEvaluator))
    {
    }


    NodeCalculator::~NodeCalculator()
    {
        delete mImpl;
        mImpl = 0;
    }


    void NodeCalculator::start()
    {
        return mImpl->start();
    }


    void NodeCalculator::stop()
    {
        return mImpl->stop();
    }


    int NodeCalculator::getCurrentSearchDepth() const
    {
        return mImpl->getCurrentSearchDepth();
    }


    int NodeCalculator::getMaxSearchDepth() const
    {
        return mImpl->getMaxSearchDepth();
    }


    NodePtr NodeCalculator::result() const
    {
        return mImpl->result();
    }


    AbstractNodeCalculator::Status NodeCalculator::status() const
    {
        return mImpl->status();
    }

} // namespace Tetris
