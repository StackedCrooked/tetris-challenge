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


    // Remove all the children from srcNode except the one on the path that leads to dstNode.
    // The children of the 'good one' are also exterminated, except the one that brings us a 
    // step closer to dstNode. This algorithm is recursively repeated until only the path between
    // srcNode and dstNode remains.
    //
    // The purpose of this function is mainly to free up memory.
    //
    void DestroyInferiorChildren(NodePtr startNode, NodePtr endNode)
    {
        NodePtr dst = endNode;
        Assert(dst->depth() > startNode->depth());
        while (dst->depth() != startNode->depth())
        {
            ChildNodes::const_iterator it = dst->parent()->children().begin(), end = dst->parent()->children().end();
            for (; it != end; ++it)
            {
                NodePtr endNode = *it;
                if (endNode == dst) // is endNode part of the path between startNode and endNode?
                {
                    // Erase all children. The endNode object is kept alive
                    // thanks to the refcounting mechanism of boost::shared_ptr.
                    dst->parent()->clearChildren();

                    // Add endNode to the child nodes again.
                    dst->parent()->addChild(endNode);
                    break;
                }
            }
            dst = dst->parent();
        }
    }


    Player::Player(std::auto_ptr<GameStateNode> inNode,
                   const BlockTypes & inBlockTypes,
                   const std::vector<size_t> & inWidths,
                   std::auto_ptr<Evaluator> inEvaluator) :
        mLayers(),
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
        setStatus(Status_Destructing);
        mThread->join();
    }
    
        
    bool Player::isFinished() const
    {
        return getStatus() == Status_Finished;;
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


    Player::Status Player::getStatus() const
    {
        boost::mutex::scoped_lock lock(mStatusMutex);
        return mStatus;
    }


    void Player::setStatus(Status inStatus)
    {
        boost::mutex::scoped_lock lock(mStatusMutex);
        mStatus = inStatus;
    }


    void Player::populateNodesRecursively(NodePtr ioNode,
                                          const BlockTypes & inBlockTypes,
                                          const std::vector<size_t> & inWidths,
                                          size_t inIndex,
                                          size_t inMaxIndex)
    {
        //
        // Check stop conditions
        //
        if (inIndex > inMaxIndex || inIndex >= inBlockTypes.size())
        {
            return;
        }


        //
        // Check status
        //
        Status status = getStatus();
        if (status == Status_Destructing || status == Status_Interrupted)
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

            // Populate ioNode and overwrite results.
            {
                Assert(inIndex <= mLayers.size());
                if (inIndex == mLayers.size())
                {
                    Protected<LayerData> newTreeInfo;
                    mLayers.push_back(newTreeInfo);
                }
                Assert(inIndex < mLayers.size());
                ScopedAtom<LayerData> scopedLayerData = mLayers[inIndex];
                LayerData & layer = *scopedLayerData.get();
                ChildNodes::iterator it = generatedChildNodes.begin(), end = generatedChildNodes.end();
                for (size_t count = 0; count < inWidths[inIndex] && it != end; ++count, ++it)
                {
                    // Populate io
                    NodePtr child = *it;
                    ioNode->addChild(child);
                    if (!layer.mBestChild || 
                        child->state().quality(child->qualityEvaluator()) > layer.mBestChild->state().quality(child->qualityEvaluator()))
                    {
                        layer.mBestChild = child;
                    }
                    layer.mNumItems++;
                }
            }
        }


        //
        // Recursive call on each child node.
        //
        if (inIndex < inMaxIndex)
        {
            for (ChildNodes::iterator it = generatedChildNodes.begin(); it != generatedChildNodes.end(); ++it)
            {
                populateNodesRecursively(*it, inBlockTypes, inWidths, inIndex + 1, inMaxIndex);
            }
        }
    }


    void Player::markTreeRowAsFinished(size_t inIndex)
    {
        size_t numLevels = mLayers.size();
        Assert(inIndex < numLevels);
        if (inIndex < numLevels)
        {
            ScopedAtom<LayerData> scopedLayerData = mLayers[inIndex];
            LayerData & layer = *scopedLayerData.get();
            layer.mFinished = true;
        }
    }


    //void create()
    //{
    //    char *buf  = new char[1000];   //pre-allocated buffer
    //    string *p = new (buf) string("hi");  //placement new
    //}


    void Player::populate()
    {
        // Use iterative-deepening
        size_t currentDepth = 0;
        size_t targetDepth = mBlockTypes.size();
        while (currentDepth < targetDepth)
        {
            populateNodesRecursively(mNode, mBlockTypes, mWidths, 0, currentDepth);
            size_t numLevels = mLayers.size();
            if (currentDepth + 1 == numLevels)
            {
                markTreeRowAsFinished(currentDepth);
            }
            else
            {
                break;
            }
            currentDepth++;
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
                DestroyInferiorChildren(mNode, layer->mBestChild);
            }
        }
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
