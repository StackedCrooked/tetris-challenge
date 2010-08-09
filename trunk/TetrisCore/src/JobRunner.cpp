#include "JobRunner.h"


namespace Tetris
{

    JobRunner::JobRunner() :
        mIsStopped(false)
    {
    }


    void JobRunner::start()
    {
        while (!isStopped())
        {
            boost::mutex::scoped_lock lock(mRunnerMutex);
            while (mJobList->empty())
            {
                mJobsAvailableCondition.wait(lock);
                mJobList->pop()();
            }
            mThread.reset(new boost::thread(mJobList->pop()));
            mThread->join();
            mThread.reset();
        }
    }


    bool JobRunner::isStopped() const
    {
        boost::mutex::scoped_lock lock(mIsStoppedMutex);
        return mIsStopped;
    }


    bool JobRunner::isIdle() const
    {
        return !mThread;
    }


    void JobRunner::push(const Job & inJob)
    {
        boost::mutex::scoped_lock lock(mRunnerMutex);
        mJobList->push(inJob);
        mJobsAvailableCondition.notify_one();
    }


} // namespace Tetris
