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
        JobList * jobs = (JobList *)outJobsAsVoidPtr;
        while (idx < cMaxChilds && it != children.end())
        {
            GameStateNode * childNode = it->get();
            Job job = boost::bind(&PopulateNode,
                                  childNode,
                                  inBlocks,
                                  inWidths,
                                  inOffset + 1,
                                  _1);
            jobs->add(job);
            ++idx;
            ++it;
        }
    }


    //void DoJobs(const JobList & inJobs)
    //{       
    //    if (inJobs.empty())
    //    {
    //        return;
    //    }

    //    JobList newJobs;
    //    for (size_t idx = 0; idx != inJobs.size(); ++idx)
    //    {
    //        const Job & job = inJobs.get(idx);

    //        // Execute the job in a new thread. Each thread
    //        // will add items to the new JobList. (Good thing
    //        // it is protected by a mutex)
    //        job((void*)&newJobs);
    //        mT
    //    }

    //    

    //    // Recursive call.
    //    // This will continue until the inOffset value
    //    // in PopulateNode reaches inBlocks.size().
    //    DoJobs(newJobs);
    //}


    const Job & JobList::get(size_t inIndex) const
    {
        boost::mutex::scoped_lock lock(mMutex);
        return mJobs[inIndex];
    }


    Job & JobList::get(size_t inIndex)
    {
        boost::mutex::scoped_lock lock(mMutex);
        return mJobs[inIndex];
    }


    void JobList::add(const Job & inJob)
    {
        boost::mutex::scoped_lock lock(mMutex);
        return mJobs.push_back(inJob);
    }


    void JobList::clear()
    {
        boost::mutex::scoped_lock lock(mMutex);
        return mJobs.clear();
    }


    size_t JobList::size() const
    {
        boost::mutex::scoped_lock lock(mMutex);
        return mJobs.size();
    }


    bool JobList::empty() const
    {
        boost::mutex::scoped_lock lock(mMutex);
        return mJobs.empty();
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
                // Call PopulateNode in a separate thread for the given job.
                // Each thread created here will add jobs to newJobs.
                localThreads.push_back(mThreadGroup.create_thread(boost::bind(inJobs.get(idx), (void*)&newJobs)));
            }
            mThreadGroup.join_all();
            std::for_each(localThreads.begin(), localThreads.end(), boost::bind(&boost::thread_group::remove_thread, &mThreadGroup, _1));
            doJobsAndWait(newJobs);
        }
    }


    void Player::move(const std::vector<int> & inWidths)
    {
        JobList jobs;
        PopulateNode(mGame->currentNode(), mGame->getFutureBlocks(inWidths.size()), inWidths, 0, (void*)&jobs);
        doJobsAndWait(jobs);
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
            Cleanup(mGame->currentNode(), child);
            mGame->setCurrentNode(child);
            if (mGame->currentNode()->depth() - printHelper >= 100)
            {
                std::cout << "Blocks: " << mGame->currentNode()->depth() << "\tLines: " << mGame->currentNode()->state().stats().mNumLines << "\r";
                printHelper = mGame->currentNode()->depth();
            }
        }        
    }


} // namespace Tetris
