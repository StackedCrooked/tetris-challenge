#include "Player.h"
#include "GameState.h"
#include "ErrorHandling.h"
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <iostream>


namespace Tetris
{
    
    
    // Remove all the children from srcNode except the one on the path that leads to dstNode.
    // The children of the 'good one' are also exterminated, except the one that brings us a 
    // step closer to dstNode. This algorithm is recursively repeated until only the path between
    // srcNode and dstNode remains.
    //
    // The purpose of this function is mainly to free up memory.
    //
    void CarvePath(GameStateNode * srcNode, GameStateNode * dstNode)
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


    //
    // Populates the given node with children. Then it continues to populate each of the children, and their children, ...
    // It ends when it has recursed as many times as (inBlocks.size() - inOffset).
    // The inWidths argument how many children should be kept. This is needed to curb the exponential explosion.
    //
    void PopulateNodesRecursively(GameStateNode * inNode,
                                  const BlockTypes & inBlocks,
                                  const std::vector<int> & inWidths,
                                  size_t inOffset)
    {

        CheckArgument(inBlocks.size() == inWidths.size(), "PopulateNodesRecursively got inBlocks.size() != inWidths.size()");
        if (inOffset >= inBlocks.size())
        {
            return;
        }

        
        if (inNode->state().isGameOver())
        {
            // Game over state has no children.
            return;
        }


        // Generate the child nodes
        GenerateOffspring(inBlocks[inOffset], *inNode, inNode->children());

        // Populate each child node.
        int idx = 0;
        const int cMaxChilds = inWidths[inOffset];
        ChildNodes & children = inNode->children();
        ChildNodes::iterator it = children.begin();
        while (idx < cMaxChilds && it != children.end())
        {
            GameStateNode & childNode = **it;
            PopulateNodesRecursively(inNode, inBlocks, inWidths, inOffset + 1);
            ++idx;
            ++it;
        }
    }


    Player::Player(Game * inGame) :
        mGame(inGame),
        mMaxThreadCount(1)
    {
    }


    void Player::setMaxThreadCount(size_t inMaxThreadCount)
    {
        mMaxThreadCount = inMaxThreadCount;
    }


    void Player::doJobsAndWait(const JobList & inJobs)
    {
        if (!inJobs.empty())
        {
            std::vector<boost::thread * > localThreads;
            localThreads.reserve(inJobs.size());
            JobList newJobs;
            for (size_t idx = 0; idx != inJobs.size(); ++idx)
            {
                // Call PopulateNodesRecursively in a separate thread for the given job.
                // Each thread created here will add jobs to newJobs.
                localThreads.push_back(mThreadGroup.create_thread(boost::bind(inJobs.get(idx), (void*)&newJobs)));
            }
            mThreadGroup.join_all();
            std::for_each(localThreads.begin(), localThreads.end(), boost::bind(&boost::thread_group::remove_thread, &mThreadGroup, _1));
            doJobsAndWait(newJobs);
        }
    }


    // Returns a copy of the given vector minus its first element.
    template<class T>
    std::vector<T> DropFirst(const std::vector<T> & inVector)
    {
        std::vector<T> result;
        for (size_t idx = 1; idx < inVector.size(); ++idx)
        {
            result.push_back(inVector[idx]);
        }
        return result;
    }


    typedef std::vector<int> Widths;
    void Player::move(const Widths & inWidths)
    {
        CheckPrecondition(!inWidths.empty(), "Player::move: depth should be at least 1.");

        // Generate the offspring.
        ChildNodes & childNodes = mGame->currentNode()->children();
        GenerateOffspring(mGame->activeBlock().type(), *mGame->currentNode(), childNodes);

        // If the dept is only 1, then we take this shortcut.
        if (inWidths.size() == 1)
        {
            GameStateNode & node = *mGame->currentNode();
            
            // Keep-alive
            ChildNodePtr bestChild = *node.children().begin();

            // Erase all children, 'bestChild' is kept alive by shared_ptr.
            node.children().clear();

            node.children().insert(bestChild);
            return;
        }


        // These variables must not be destroyed until after the 'threadPool.join_all()' below.
        std::vector<boost::shared_ptr<BlockTypes> > keepAlive_BlockTypes;
        std::vector<boost::shared_ptr<Widths> > keepAlive_Widths;
        std::vector<boost::shared_ptr<GameStateNode> > keepAlive_GameStateNodes;
        boost::thread_group threadPool;

        int count = 0;
        const int cMaxChildCount = inWidths[0];
        ChildNodes::iterator it = childNodes.begin(), end = childNodes.end();
        while (it != end && count != cMaxChildCount)
        {
            GameStateNode & node = **it;
            
            // WARNING!
            // The PopulateNodesRecursively method takes const ref arguments. We must make sure
            // any arguments lifetime will last until after the threadPool.join_all call below.

            // An BIG advantage however is that each thread will have its own copy of the data structures.
            // This leads to greatly reduced interthread synchronization, which in turn leads to much
            // more efficient CPU utilization and thus better performance.
            keepAlive_BlockTypes.push_back(boost::shared_ptr<BlockTypes>(new BlockTypes(mGame->getFutureBlocks(inWidths.size()))));
            keepAlive_Widths.push_back(boost::shared_ptr<Widths>(new Widths(inWidths)));
            
            // Call the PopulateNodesRecursively function in a separate thread.
            std::auto_ptr<GameStateNode> clonedGameState(node.clone());
            threadPool.create_thread(
                boost::bind(&PopulateNodesRecursively,
                            clonedGameState.get(),
                            *keepAlive_BlockTypes.back(),
                            *keepAlive_Widths.back(),
                            1));
            
            // Give owner-ship of the cloned game-state to the keepAlive object.
            keepAlive_GameStateNodes.push_back(boost::shared_ptr<GameStateNode>(clonedGameState.release()));
            count++;
            ++it;
        }

        // Wait for all threads to complete.
        threadPool.join_all();
    }


    void Player::playUntilGameOver(const std::vector<int> & inDepths)
    {
        int printHelper = mGame->currentNode()->depth();
        while (!mGame->isGameOver())
        {
            move(inDepths);
            size_t depth = inDepths.size();
            GameStateNode * child = mGame->currentNode()->bestChild(depth);
            while (!child && depth > 0) // in case of game-over this can happen
            {
                child = mGame->currentNode()->bestChild(depth--);
            }
            
            CarvePath(mGame->currentNode(), child);
            mGame->setCurrentNode(child);
            if (mGame->currentNode()->depth() - printHelper >= 100)
            {
                std::cout << "Blocks: " << mGame->currentNode()->depth() << "\tLines: " << mGame->currentNode()->state().stats().numLines() << "\r";
                printHelper = mGame->currentNode()->depth();
            }
        }        
    }


} // namespace Tetris
