#include "Tetris/Player.h"
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

    Player::Player(std::auto_ptr<GameStateNode> inNode,
                   const BlockTypes & inBlockTypes,
                   const Widths & inWidths,
                   std::auto_ptr<Evaluator> inEvaluator) :
        mLayers(inBlockTypes.size()),
        mCompletedSearchDepth(Create<int>(0)),
        mNode(inNode.release()),
        mBlockTypes(inBlockTypes),
        mWidths(inWidths),
        mEvaluator(inEvaluator),
        mStatus(Status_Null),
        mThread()
    {
        Assert(!mNode->state().isGameOver());
        Assert(mNode->children().empty());
        mNode->makeRoot(); // forget out parent node
        mStopwatch.reset(new Poco::Stopwatch);
        mStopwatch->start();
    }


    Player::~Player()
    {
        mThread->interrupt();
        setStatus(Status_Interrupted);
        mThread->join();
    }
    
        
    bool Player::isFinished() const
    {
        return status() == Status_Finished;
    }


    int Player::getCurrentSearchDepth() const
    {
        ScopedConstAtom<int> value(mCompletedSearchDepth);
        return *value.get();
    }


    int Player::getMaxSearchDepth() const
    {
        return mWidths.size();
    }


    bool Player::result(NodePtr & outChild)
    {
        if (mNode->children().size() == 1)
        {
            outChild = *mNode->children().begin();
            return true;
        }
        return false;
    }


    Player::Status Player::status() const
    {
        boost::mutex::scoped_lock lock(mStatusMutex);
        return mStatus;
    }


    void Player::setStatus(Status inStatus)
    {
        boost::mutex::scoped_lock lock(mStatusMutex);
        mStatus = inStatus;
    }


    void Player::updateLayerData(size_t inIndex, NodePtr inNodePtr, size_t inCount)
    {        
        ScopedAtom<LayerData> wLayerData = mLayers[inIndex];
        LayerData & layerData = *wLayerData.get();
        layerData.mNumItems += inCount;        
        if (!layerData.mBestChild || inNodePtr->state().quality(inNodePtr->qualityEvaluator()) > layerData.mBestChild->state().quality(inNodePtr->qualityEvaluator()))
        {
            layerData.mBestChild = inNodePtr;
        }
    }


    void Player::populateNodesRecursively(NodePtr ioNode,
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

         
            size_t count = 0;
            ChildNodes::iterator it = generatedChildNodes.begin(), end = generatedChildNodes.end();
            while (count < inWidths[inIndex] && it != end)
            {
                boost::this_thread::interruption_point();
                ioNode->addChild(*it);
                ++count;
                ++it;
            }

            updateLayerData(inIndex, *ioNode->children().begin(), count);
        }


        //
        // Recursive call on each child node.
        //
        if (inIndex <= inMaxIndex)
        {
            for (ChildNodes::iterator it = generatedChildNodes.begin(); it != generatedChildNodes.end(); ++it)
            {
                populateNodesRecursively(*it, inBlockTypes, inWidths, inIndex + 1, inMaxIndex);
            }
        }
    }


    void Player::markTreeRowAsFinished(size_t inIndex)
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


    void Player::populate()
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
            LogInfo("Player::populate: thread interrupted");
        }
        catch (const std::exception & inException)
        {
            LogError(MakeString() << "Player::populate: " << inException.what());
        }
    }


    void Player::destroyInferiorChildren()
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


    void Player::interrupt()
    {
        setStatus(Status_Interrupted);
        mThread->interrupt();
    }


    void Player::startImpl()
    {
        // Thread entry point has try/catch block
        try
        {
            setStatus(Status_Calculating);
            populate();
            destroyInferiorChildren();
            setStatus(Status_Finished);
        } 
        catch (const std::exception & inException)
        {
            LogError(MakeString() << inException.what());
        }
    }


    void Player::start()
    {
        Assert(!mThread);
        if (!mThread)
        {
            mThread.reset(new boost::thread(boost::bind(&Player::startImpl, this)));
        }
    }


} // namespace Tetris
