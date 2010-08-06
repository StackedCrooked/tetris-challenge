#include "Player.h"
#include "GameState.h"
#include "ErrorHandling.h"
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <iostream>


namespace Tetris
{
    
    
    void Cleanup(GameStateNode * currentNode, GameStateNode * child)
    {
        GameStateNode * immediateChildParent = child->parent();
        GameStateNode * immediateChild = child;
        while (immediateChild->depth() - currentNode->depth() > 0)
        {
            ChildNodes::iterator it = immediateChildParent->children().begin(), end = immediateChildParent->children().end();
            for (; it != end; ++it)
            {
                ChildNodePtr child = *it;
                if (child.get() == immediateChild)
                {
                    immediateChildParent->children().clear();
                    immediateChildParent->children().insert(child);
                    break;
                }
            }
            immediateChild = immediateChild->parent();
            immediateChildParent = immediateChild->parent();
        }
    }


    void PopulateNode(GameStateNode * inNode,
                      const BlockTypes & inBlocks,
                      const std::vector<int> & inWidths,
                      size_t inOffset,
                      void * outJobsAsVoidPtr)
    {
        CheckArgument(inBlocks.size() == inWidths.size(), "PopulateNode got inBlocks.size() != inWidths.size()");
        if (inOffset >= inBlocks.size())
        {
            return;
        }

        
        if (inNode->state().isGameOver())
        {
            // Game over state has no children.
            return;
        }


        // Generate the child nodes for inNode
        GenerateOffspring(inBlocks[inOffset], *inNode, inNode->children());

        // Populate each child node.
        int idx = 0;
        const int cMaxChilds = inWidths[inOffset];
        ChildNodes & children = inNode->children();
        ChildNodes::iterator it = children.begin();
        Jobs * jobs = (Jobs *)outJobsAsVoidPtr;
        while (idx < cMaxChilds && it != children.end())
        {
            GameStateNode * childNode = it->get();
            Job job = boost::bind(&PopulateNode,
                                  childNode,
                                  inBlocks,
                                  inWidths,
                                  inOffset + 1,
                                  _1);
            jobs->push_back(job);
            ++idx;
            ++it;
        }
    }


    void DoJobs(const Jobs & inJobs)
    {       
        if (inJobs.empty())
        {
            return;
        }

        Jobs newJobs;
        for (size_t idx = 0; idx != inJobs.size(); ++idx)
        {
            const Job & job = inJobs[idx];

            // Execute the job, and fetch new jobs.
            job((void*)&newJobs);
        }

        // Recursive call.
        // This will continue until the inOffset value
        // in PopulateNode reaches inBlocks.size().
        DoJobs(newJobs);
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


    void Player::move(const std::vector<int> & inWidths)
    {
        Jobs jobs;
        PopulateNode(mGame->currentNode(), mGame->getFutureBlocks(inWidths.size()), inWidths, 0, (void*)&jobs);
        DoJobs(jobs);
        
    }


    void Player::playUntilGameOver(const std::vector<int> & inDepths)
    {
        while (!mGame->isGameOver())
        {
            move(inDepths);
            size_t depth = inDepths.size();
            GameStateNode * child = mGame->currentNode()->bestChild(depth);
            while (!child && depth > 0) // if we didn't reach the requested depth, for some reason??
            {
                CheckCondition(false, "Actually, we should never come here.");
                child = mGame->currentNode()->bestChild(depth--);
            }
            
            Cleanup(mGame->currentNode(), child);

            mGame->setCurrentNode(child);
        }
    }


} // namespace Tetris
