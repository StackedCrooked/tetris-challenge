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
        mCompletedSearchDepth(Create<int>(0)),
        mNode(inNode.release()),
        mBlockTypes(inBlockTypes),
        mWidths(inWidths),
        mEvaluator(inEvaluator),
        mStatus(Status_Nil)
    {
        Assert(!mNode->state().isGameOver());
        Assert(mNode->children().empty());
        mNode->makeRoot(); // forget out parent node
        mStopwatch.reset(new Poco::Stopwatch);
        mStopwatch->start();
    }


    ComputerPlayer::~ComputerPlayer()
    {
        // Remove any obsolete tasks.
        mWorkerThread->clear();
        mWorkerThread->interrupt();
        setStatus(Status_Interrupted);
    }
    
        
    bool ComputerPlayer::isFinished() const
    {
        return status() == Status_Finished;
    }


    int ComputerPlayer::getCurrentSearchDepth() const
    {
        ScopedConstAtom<int> value(mCompletedSearchDepth);
        return *value.get();
    }


    int ComputerPlayer::getMaxSearchDepth() const
    {
        return mWidths.size();
    }


    bool ComputerPlayer::result(NodePtr & outChild)
    {
        if (mNode->children().size() == 1)
        {
            outChild = *mNode->children().begin();
            return true;
        }
        return false;
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
        ScopedAtom<LayerData> wLayerData = mLayers[inIndex];
        LayerData & layerData = *wLayerData.get();
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
        // Throws boost::thread_interrupted if boost::thread::interrupt() has been called.
        boost::this_thread::interruption_point();

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
            boost::this_thread::interruption_point();

         
            size_t count = 0;
            ChildNodes::iterator it = generatedChildNodes.begin(), end = generatedChildNodes.end();
            while (count < inWidths[inIndex] && it != end)
            {
                ioNode->addChild(*it);
                ++count;
                ++it;
                boost::this_thread::interruption_point();
            }

            updateLayerData(inIndex, *ioNode->children().begin(), count);
            boost::this_thread::interruption_point();
        }


        //
        // Recursive call on each child node.
        //
        if (inIndex <= inMaxIndex)
        {
            for (ChildNodes::iterator it = generatedChildNodes.begin(); it != generatedChildNodes.end(); ++it)
            {
                NodePtr child = *it;
                populateNodesRecursively(child, inBlockTypes, inWidths, inIndex + 1, inMaxIndex);
                boost::this_thread::interruption_point();
            }
        }
    }


    void ComputerPlayer::markTreeRowAsFinished(size_t inIndex)
    {
        {
            ScopedAtom<int> wdepth(mCompletedSearchDepth);
            int & depth = *wdepth.get();
            if (inIndex > depth)
            {
                depth = inIndex;
            }
        }

        ScopedAtom<LayerData> layerData(mLayers[inIndex]);
        layerData->mFinished = true;
    }


    void ComputerPlayer::populate()
    {
        try
        {
            // The nodes are populated using a simple "Iterative deepening" algorithm.
            // See: http://en.wikipedia.org/wiki/Iterative_deepening_depth-first_search for more information.
            size_t targetDepth = 1;
            while (targetDepth < mBlockTypes.size())
            {
                populateNodesRecursively(mNode, mBlockTypes, mWidths, 0, targetDepth);
                markTreeRowAsFinished(targetDepth);
                targetDepth++;
            }
        }
        catch (const boost::thread_interrupted &)
        {
            LogInfo("ComputerPlayer::populate: thread interrupted");
        }
        catch (const std::exception & inException)
        {
            LogError(MakeString() << "ComputerPlayer::populate: " << inException.what());
        }
    }


    void ComputerPlayer::destroyInferiorChildren()
    {
        if (!mLayers.empty())
        {
            LayerData * layer(0);
            for (size_t idx = 0; idx != mLayers.size(); ++idx)
            {
                ScopedAtom<LayerData> scopedLayerData = mLayers[idx];
                if (!scopedLayerData->mFinished)
                {
                    break;
                }
                layer = scopedLayerData.get();
            }
            if (layer)
            {
                CarveBestPath(mNode, layer->mBestChild);
            }
        }
    }


    void ComputerPlayer::interrupt()
    {
        setStatus(Status_Interrupted);
        mWorkerThread->interrupt();
    }


    void ComputerPlayer::startImpl()
    {
        // Thread entry point has try/catch block
        try
        {
            setStatus(Status_Started);
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
        Assert(status() == Status_Nil);
        mWorkerThread->schedule(boost::bind(&ComputerPlayer::startImpl, this));
    }

} // namespace Tetris
