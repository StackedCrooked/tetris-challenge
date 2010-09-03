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
    void DestroyInferiorChildren(GameStateNode & startNode, GameStateNode & endNode)
    {
        GameStateNode * dst = &endNode;
        Assert(dst->depth() > startNode.depth());
        while (dst->depth() != startNode.depth())
        {
            ChildNodes::const_iterator it = dst->parent()->children().begin(), end = dst->parent()->children().end();
            for (; it != end; ++it)
            {
                ChildNodePtr endNode = *it;
                if (endNode.get() == dst) // is endNode part of the path between startNode and endNode?
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
                   std::auto_ptr<Evaluator> inEvaluator) :
        mTreeRows(),
        mNode(inNode.release()),
        mBlockTypes(inBlockTypes),
        mEvaluator(inEvaluator),
        mStatus(Status_Null),
        mThread()
    {
        Assert(!mNode->state().isGameOver());
        Assert(mNode->children().empty());
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


    bool Player::isGameOver() const
    {
        if (!mNode->children().empty())
        {
            return (*mNode->children().begin())->state().isGameOver();
        }
        return false;
    }


    bool Player::result(ChildNodePtr & outChild)
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


    void Player::populateNodesRecursively(GameStateNode & ioNode,
                                          const BlockTypes & inBlockTypes,
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


        if (ioNode.state().isGameOver())
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
        ChildNodes generatedChildNodes = ioNode.children();
        if (generatedChildNodes.empty())
        {
            generatedChildNodes = ChildNodes(GameStateComparisonFunctor(mEvaluator->clone()));
            GenerateOffspring(inBlockTypes[inIndex], ioNode, *mEvaluator, generatedChildNodes);

            // Populate ioNode and overwrite results.
            {
                Assert(inIndex <= mTreeRows.size());
                if (inIndex == mTreeRows.size())
                {
                    Protected<TreeRowInfo> newTreeInfo;
                    mTreeRows.push_back(newTreeInfo);
                }
                Assert(inIndex < mTreeRows.size());
                ScopedAtom<TreeRowInfo> scopedTreeRowInfo = mTreeRows[inIndex];
                TreeRowInfo & rowInfo = *scopedTreeRowInfo.get();
                ChildNodes::iterator it = generatedChildNodes.begin(), end = generatedChildNodes.end();
                for (; it != end; ++it)
                {
                    // Populate io
                    ChildNodePtr child = *it;
                    ioNode.addChild(child);
                    if (!rowInfo.mBestChild || 
                        child->state().quality(child->qualityEvaluator()) > rowInfo.mBestChild->state().quality(child->qualityEvaluator()))
                    {
                        rowInfo.mBestChild = child;
                    }
                    rowInfo.mNumItems++;
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
                populateNodesRecursively(**it, inBlockTypes, inIndex + 1, inMaxIndex);
            }
        }
    }


    void Player::markTreeRowAsFinished(size_t inIndex)
    {
        size_t numTreeRows = mTreeRows.size();
        Assert(inIndex < numTreeRows);
        if (inIndex < numTreeRows)
        {
            ScopedAtom<TreeRowInfo> scopedTreeRowInfo = mTreeRows[inIndex];
            TreeRowInfo & rowInfo = *scopedTreeRowInfo.get();
            rowInfo.mFinished = true;
        }
    }


    void Player::populateBreadthFirst()
    {
        size_t maxIndex = 0;
        size_t numBlockTypes = mBlockTypes.size();
        while (maxIndex < numBlockTypes)
        {
            populateNodesRecursively(*mNode, mBlockTypes, 0, maxIndex);
            size_t numTreeRows = mTreeRows.size();
            if (maxIndex + 1 == numTreeRows)
            {
                markTreeRowAsFinished(maxIndex);
            }
            else
            {
                break;
            }
            maxIndex++;
        }
    }


    void Player::destroyInferiorChildren()
    {
        if (!mTreeRows.empty())
        {
            TreeRowInfo * rowInfo(0);
            for (size_t idx = 0; idx != mTreeRows.size(); ++idx)
            {
                ScopedAtom<TreeRowInfo> scopedTreeRowInfo = mTreeRows[idx];
                if (!scopedTreeRowInfo->mFinished)
                {
                    break;
                }
                rowInfo = scopedTreeRowInfo.get();
            }
            if (rowInfo)
            {
                DestroyInferiorChildren(*mNode, *rowInfo->mBestChild);
            }
        }
    }


    void Player::startImpl()
    {
        // Thread entry point has try/catch block
        try
        {
            setStatus(Status_Calculating);
            populateBreadthFirst();
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
