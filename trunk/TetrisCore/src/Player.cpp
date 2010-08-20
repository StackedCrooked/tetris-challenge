#include "Player.h"
#include "GameState.h"
#include "ErrorHandling.h"
#include "Logger.h"
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
    void DestroyInferiorChildren(GameStateNode * srcNode, GameStateNode * dstNode)
    {
        CheckPrecondition(srcNode != NULL, "Goddamn.");
        CheckPrecondition(dstNode != NULL, "Goddamn.");
        GameStateNode * dstParent = dstNode->parent();
        GameStateNode * dst = dstNode;
        while (dst->depth() - srcNode->depth() > 0)
        {
            ChildNodes::iterator it = dstParent->children().begin(), end = dstParent->children().end();
            for (; it != end; ++it)
            {
                ChildNodePtr dstNode = *it;
                if (dstNode.get() == dst) // is dstNode part of the path between srcNode and dstNode?
                {
                    // Erase all children. The dstNode object is kept alive
                    // thanks to the refcounting mechanism of boost::shared_ptr.
                    dstParent->children().clear();

                    // Add dstNode to the child nodes again.
                    dstParent->children().insert(dstNode);
                    break;
                }
            }
            dst = dst->parent();
            dstParent = dst->parent();
        }
    }


    TimedNodePopulator::TimedNodePopulator(std::auto_ptr<GameStateNode> inNode,
                                           const BlockTypes & inBlockTypes,
                                           int inTimeMs) :
        mNode(inNode.release()),
        mBlockTypes(inBlockTypes),
        mTimeMicroseconds(1000 * inTimeMs),
        mTimer(inTimeMs, 0),
        mIsTimeExpired(false)
    {
        if (mBlockTypes.size() > cMaxDepth)
        {
            LogWarning(MakeString() << "The number of blocks (" << mBlockTypes.size() << ") exceeds the maximum depth (" << cMaxDepth << "). They will be ignored.");
        }
    }
        
        
    void TimedNodePopulator::onTimer(Poco::Timer &)
    {
        mIsTimeExpired = true;
    }


    size_t TimedNodePopulator::getCurrentDepth() const
    {
        size_t idx = cMaxDepth - 1;
        while (idx >= 0)
        {
            ScopedConstAtom<Result> result(mResult);
            if (result->sizeAtDepth(idx) != 0)
            {
                break;
            }
            idx--;
        }
        return idx;
    }


    ChildNodePtr TimedNodePopulator::getBestChild() const
    {
        int currentDepth = getCurrentDepth();
        ScopedConstAtom<Result> result(mResult);
        return *(result->getNodesAtDepth(currentDepth).begin());
    }


    void TimedNodePopulator::addToFlattenedNodes(const ChildNodes & inChildNodes, size_t inDepth)
    {        
        if (inDepth >= cMaxDepth)
        {
            std::string message(MakeString() << "Tried to exceed max node depth " << inDepth << ". Max entries is " << cMaxDepth << ".");
            throw std::out_of_range(message.c_str());
        }

        if (!mThreadLocalResult.get())
        {
            mThreadLocalResult.reset(new Result);
        }
        mThreadLocalResult->mergeAtDepth(inDepth, inChildNodes);
    }


    void TimedNodePopulator::commitThreadLocalData()
    {
        if (!mThreadLocalResult.get())
        {
            return;
        }

        ScopedAtom<Result> result(mResult);
        
        for (size_t idx = 0; idx != cMaxDepth; ++idx)
        {
            result->mergeAtDepth(idx, mThreadLocalResult->getNodesAtDepth(idx));
        }

        mThreadLocalResult.reset();
    }


    bool TimedNodePopulator::isTimeExpired()
    {
        return mIsTimeExpired;
    }

    
    namespace Counters
    {
        static int fCounter = 0;
        static boost::mutex fCounterMutex;
    }


    void TimedNodePopulator::populateNodesRecursively(GameStateNode & ioNode, const BlockTypes & inBlockTypes, size_t inDepth)
    {
        if (isTimeExpired())
        {
            return;
        }

        if (inDepth >= inBlockTypes.size() || inDepth >= cMaxDepth)
        {
            return;
        }
        
        if (ioNode.state().isGameOver())
        {
            // Game over state has no children.
            return;
        }

        // Generate the child nodes
        GenerateOffspring(inBlockTypes[inDepth], ioNode, ioNode.children());
        ChildNodes & children = ioNode.children();

        // Store the fruit of our labor.
        addToFlattenedNodes(children, inDepth);

        // Recursion.
        for (ChildNodes::iterator it = children.begin(); it != children.end(); ++it)
        {
            GameStateNode & childNode = **it;
            populateNodesRecursively(childNode, inBlockTypes, inDepth + 1);

            // This is spielerei: commit "now and then"
            {
                bool doIt = false;

                // Critical section
                {
                    boost::mutex::scoped_lock lock(Counters::fCounterMutex);
                    Counters::fCounter++;
                    if (Counters::fCounter % 1000 == 0)
                    {
                        doIt = true;
                        Counters::fCounter = 0;
                    }
                }

                if (doIt)
                {
                    commitThreadLocalData();
                }
            }
        }
    }


    void TimedNodePopulator::populateNodesInBackground(ChildNodePtr inNode, BlockTypes * inBlockTypes, size_t inDepth)
    {
        // Thread entry point
        try
        {
            boost::scoped_ptr<BlockTypes> takeOwnership(inBlockTypes);
            populateNodesRecursively(*inNode, *inBlockTypes, inDepth);
            commitThreadLocalData();
       } 
        catch (const std::exception & inException)
        {
            LogError(MakeString() << "populateNodesInBackground failed: " << inException.what());
        }
    }
        
    
    void TimedNodePopulator::start()
    {
        Poco::TimerCallback<TimedNodePopulator> timerCallback(*this, &TimedNodePopulator::onTimer);
        mTimer.start(timerCallback);

        // Generate children of the root node
        GenerateOffspring(mBlockTypes[0], *mNode, mNode->children());

        // For each child node:
        ChildNodes::const_iterator it = mNode->children().begin(), end = mNode->children().end();
        for (; it != end; ++it)
        {
            ChildNodePtr firstGenerationChildNode = *it;
            mThreadPool.create_thread(
                boost::bind(&TimedNodePopulator::populateNodesInBackground,
                            this,
                            firstGenerationChildNode,
                            new BlockTypes(mBlockTypes),
                            1));
        }

        mThreadPool.join_all();
    }


} // namespace Tetris
