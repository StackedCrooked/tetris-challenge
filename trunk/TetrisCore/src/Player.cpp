#include "Player.h"
#include "GameState.h"
#include "ErrorHandling.h"
#include "Poco/Stopwatch.h"
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
                      BlockTypes inBlocks,              // No const-ref here because I want to minimize sharing, for now...
                      std::vector<int> inWidths,
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

    
    
    // Thread locals
    boost::condition_variable mtJobDone;
    boost::thread_specific_ptr<JobList> mtJobList;

    class Worker
    {
    public:
        Worker() :
            mDone(false)
        {
        }        

        bool done() const
        {
            boost::mutex::scoped_lock lock(mDoneMutex);
            return mDone;
        }

        // Worker thread
        void work(Job inJob)
        {
            mtJobList.reset(new JobList);
            mtJobList->add(inJob);

            for (size_t idx = 0; idx < mtJobList->size(); ++idx)
            {
                const Job & job = mtJobList->get(idx);
                job((void*)mtJobList.get());
            }

            // Update the mDone value.
            {
                boost::mutex::scoped_lock lock(mDoneMutex);
                mDone = true;
            }
            mtJobDone.notify_one();
        }

    private:
        mutable boost::mutex mDoneMutex;
        bool mDone;
    };

    const int cMaxThreads = 20;
    typedef std::vector<boost::shared_ptr<Worker> > Workers;
    
    
    int ActiveWorkerCount(const Workers & inWorkers)
    {
        int result = inWorkers.size();
        for (size_t idx = 0; idx != inWorkers.size(); ++idx)
        {
            if (inWorkers[idx]->done())
            {
                result--;
            }
        }
        return result;
    }


    void Player::doJobsAndWait(const JobList & inJobs)
    {
        
        
        boost::thread_group threadGroup;
        Workers workers;
        for (size_t idx = 0; idx < inJobs.size(); ++idx)
        {
            {
                boost::mutex here_is_a_mutex;
                boost::mutex::scoped_lock this_is_a_lock(here_is_a_mutex);
                mtJobDone.wait(this_is_a_lock, boost::bind(&ActiveWorkerCount, workers) < cMaxThreads);
            }
            const Job & job(inJobs.get(idx));
            workers.push_back(boost::shared_ptr<Worker>(new Worker));
            Worker * worker(workers.back().get());            
            threadGroup.create_thread(boost::bind(&Worker::work, worker, job));
        }
        threadGroup.join_all();
    }


    void Player::move(const std::vector<int> & inWidths)
    {
        JobList jobs;
        PopulateNode(mGame->currentNode(), mGame->getFutureBlocks(inWidths.size()), inWidths, 0, (void*)&jobs);
        doJobsAndWait(jobs);
    }


    void Player::playUntilGameOver(const std::vector<int> & inDepths)
    {
        Poco::Stopwatch stopWatch;
        stopWatch.start();
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

            if (stopWatch.elapsed() > 100 * 1000)
            {
                std::cout << "Blocks: " << mGame->currentNode()->depth() << "\tLines: " << mGame->currentNode()->state().stats().mNumLines << "\r";
                stopWatch.restart();
            }
        }        
    }


} // namespace Tetris
