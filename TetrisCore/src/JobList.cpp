#include "JobList.h"
#include <boost/bind.hpp>


namespace Tetris
{


    JobList::JobList()        
    {
        mThread.reset(new boost::thread(boost::bind(&JobList::start, this)));
    }

    void JobList::start()
    {
        while (true)
        {
            boost::shared_ptr<Job> job;
            {
                boost::mutex::scoped_lock lock(mMutex);
                while (mJobs.empty())
                {
                    mJobsInQueue.wait(lock);
                }
                job = mJobs.front();
                mJobs.pop();
            }
            (*job)();
        }
    }


    //int JobList::push(std::auto_ptr<Job> inJob)
    //{
    //    boost::mutex::scoped_lock lock(mMutex);
    //    mJobs.push(boost::shared_ptr<Job>(inJob.release()));
    //    mJobsInQueue.notify_one();
    //}


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


} // namespace Tetris
