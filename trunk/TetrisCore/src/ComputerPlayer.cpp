#include "Tetris/ComputerPlayer.h"
#include "Tetris/GameState.h"
#include "Tetris/ErrorHandling.h"
#include "Tetris/Logger.h"
#include "Poco/Stopwatch.h"
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <ostream>


namespace Tetris
{

    ComputerPlayer::ComputerPlayer(boost::shared_ptr<WorkerThread> inWorkerThread,
                                   std::auto_ptr<GameStateNode> inNode,
                                   const BlockTypes & inBlockTypes,
                                   const Widths & inWidths,
                                   std::auto_ptr<Evaluator> inEvaluator) :
        mWorkerThread(inWorkerThread),
        mLayers(inBlockTypes.size()),
        mCompletedSearchDepth(0),
        mNode(inNode.release()),
        mBlockTypes(inBlockTypes),
        mWidths(inWidths),
        mEvaluator(inEvaluator),
        mStatus(Status_Nil)
    {
        Assert(!mNode->state().isGameOver());
        Assert(mNode->children().empty());
        mNode->makeRoot(); // forget out parent node
    }


    ComputerPlayer::~ComputerPlayer()
    {
        mWorkerThread->interrupt();
    }
    
        
    bool ComputerPlayer::isFinished() const
    {
        return status() == Status_Finished;
    }


    int ComputerPlayer::getCurrentSearchDepth() const
    {
        boost::mutex::scoped_lock lock(mCompletedSearchDepthMutex);
        return mCompletedSearchDepth;
    }
    
    
    void ComputerPlayer::setCurrentSearchDepth(int inDepth)
    {
        boost::mutex::scoped_lock lock(mCompletedSearchDepthMutex);
        if (inDepth > mCompletedSearchDepth)
        {
            mCompletedSearchDepth = inDepth;
        }
    }


    int ComputerPlayer::getMaxSearchDepth() const
    {
        return mWidths.size();
    }


    NodePtr ComputerPlayer::result() const
    {
        if (status() != Status_Finished)
        {
            throw std::logic_error("ComputerPlayer::result() was called while not yet finished.");
        }

        boost::mutex::scoped_lock lock(mNodeMutex);
        if (mNode->children().empty())
        {
            throw std::runtime_error("The computer player failed to generate any results.");
        }

        size_t numChildren = mNode->children().size();
        Assert(numChildren == 1);

        return *mNode->children().begin();
    }


    ComputerPlayer::Status ComputerPlayer::status() const
    {
        boost::mutex::scoped_lock lock(mStatusMutex);
        return mStatus;
    }


    void ComputerPlayer::setStatus(Status inStatus)
    {
        boost::mutex::scoped_lock lock(mStatusMutex);
        mStatus = inStatus;
    }


    void ComputerPlayer::updateLayerData(size_t inIndex, NodePtr inNodePtr, size_t inCount)
    {
        boost::mutex::scoped_lock lock(mLayersMutex);
        LayerData & layerData = mLayers[inIndex];
        layerData.mNumItems += inCount;        
        if (!layerData.mBestChild || inNodePtr->state().quality(inNodePtr->qualityEvaluator()) > layerData.mBestChild->state().quality(inNodePtr->qualityEvaluator()))
        {
            layerData.mBestChild = inNodePtr;
        }
    }


    void ComputerPlayer::populateNodesRecursively(NodePtr ioNode,
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
         
            size_t count = 0;
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


    void ComputerPlayer::markTreeRowAsFinished(size_t inIndex)
    {
        setCurrentSearchDepth(inIndex + 1);
        boost::mutex::scoped_lock lock(mLayersMutex);
        mLayers[inIndex].mFinished = true;
    }


    void ComputerPlayer::populate()
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
            LogInfo(MakeString() << "ComputerPlayer was interrupted at search depth: " << getCurrentSearchDepth() << "/" << mBlockTypes.size());
        }
        catch (const std::exception & inException)
        {
            LogError(MakeString() << "Exception caught in ComputerPlayer::populate(). Detail: " << inException.what());
        }
    }


    void ComputerPlayer::destroyInferiorChildren()
    {
        int reachedDepth = getCurrentSearchDepth();
        Assert(reachedDepth >= 1);
        
        // We use the 'best child' from this search depth.
        // The path between the start node and this best
        // child will be the list of precalculated nodes.
        boost::mutex::scoped_lock layersLock(mLayersMutex);
        boost::mutex::scoped_lock nodeLock(mNodeMutex);
        Assert((reachedDepth - 1) < mLayers.size());
        CarveBestPath(mNode, mLayers[reachedDepth - 1].mBestChild);
    }


    void ComputerPlayer::stop()
    {
        mWorkerThread->interrupt();
    }


    void ComputerPlayer::startImpl()
    {
        // Thread entry point has try/catch block
        try
        {
            populate();
            destroyInferiorChildren();
            setStatus(Status_Finished);
        }
        catch (const std::exception & inException)
        {
            LogError(MakeString() << inException.what());
        }
    }


    void ComputerPlayer::start()
    {
        boost::mutex::scoped_lock lock(mStatusMutex);
        Assert(mStatus == Status_Nil);
        mWorkerThread->schedule(boost::bind(&ComputerPlayer::startImpl, this));
        mStatus = Status_Started;
    }

} // namespace Tetris
