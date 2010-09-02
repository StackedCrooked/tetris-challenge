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
                   std::auto_ptr<Evaluator> inEvaluator,
                   int inTimeLimitMs) :
        mNode(inNode.release()),
        mEndNode(),
        mBlockTypes(inBlockTypes),
        mEvaluator(inEvaluator),
        mTimeLimitMs(inTimeLimitMs),
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


    void Player::populateNodesRecursively(GameStateNode & ioNode, const BlockTypes & inBlockTypes, size_t inDepth)
    {
        //
        // Check status
        //
        Status status = getStatus();
        if (status == Status_Destructing || status == Status_TimeExpired)
        {
            return;
        }

        
        //
        // Check remaining time
        //
        int remainingTime = timeRemaining();
        if (remainingTime <= 0)
        {
            setStatus(Status_TimeExpired);
            return;
        }


        //
        // Check stop condition
        //
        if (inDepth >= inBlockTypes.size())
        {
            return;
        }
        
        if (ioNode.state().isGameOver())
        {
            // Game over state has no children.
            return;
        }


        //
        // Generate the child nodes
        //
        ChildNodes generatedChildNodes(GameStateComparisonFunctor(mEvaluator->clone()));
        GenerateOffspring(inBlockTypes[inDepth], ioNode, *mEvaluator, generatedChildNodes);


        //
        // Populate ioNode and overwrite mEndNode.
        //
        ChildNodes::iterator it = generatedChildNodes.begin(), end = generatedChildNodes.end();
        for (; it != end; ++it)
        {
            // Populate io
            ChildNodePtr child = *it;
            ioNode.addChild(child);
            if (inDepth == inBlockTypes.size() - 1)
            {
                if (!mEndNode)
                {
                    mEndNode = child;
                }
                else if (child->state().quality(child->qualityEvaluator()) > mEndNode->state().quality(child->qualityEvaluator()))
                {
                    mEndNode = child;
                }
            }
        }

        //
        // Recursive call on each child node.
        //
        for (ChildNodes::iterator it = generatedChildNodes.begin(); it != generatedChildNodes.end(); ++it)
        {
            populateNodesRecursively(**it, inBlockTypes, inDepth + 1);
        }
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
        if (!mNode->children().empty())
        {
            outChild = *mNode->children().begin();
            return true;
        }
        return false;
    }
    
    
    int Player::timeRemaining() const
    {
        boost::mutex::scoped_lock lock(mStopwatchMutex);
        int elapsedMs = static_cast<int>((mStopwatch->elapsed() / static_cast<Poco::Timestamp::TimeDiff>(1000)));
        int timeRemaining = mTimeLimitMs - elapsedMs;
        return timeRemaining;
    }


    void Player::setTimeExpired()
    {
        setStatus(Status_TimeExpired);
    }


    void Player::start()
    {
        Assert(!mThread);
        if (!mThread)
        {
            mThread.reset(new boost::thread(boost::bind(&Player::startImpl, this)));
        }
    }


    void Player::startImpl()
    {
        // Thread entry point has try/catch block
        try
        {
            setStatus(Status_Calculating);
            populateNodesRecursively(*mNode, mBlockTypes, 0);
            if (mEndNode)
            {
                DestroyInferiorChildren(*mNode, *mEndNode);
            }
            setStatus(Status_Finished);
        } 
        catch (const std::exception & inException)
        {
            LogError(MakeString() << inException.what());
        }
    }


} // namespace Tetris
