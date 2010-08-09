#include "JobList.h"


namespace Tetris
{


    const Job & JobList::peek() const
    {
        boost::mutex::scoped_lock lock(mMutex);
        return mJobs.front();
    }


    Job JobList::pop()
    {
        boost::mutex::scoped_lock lock(mMutex);
        Job result = mJobs.front();
        mJobs.pop();
        return result;
    }


    void JobList::push(const Job & inJob)
    {
        boost::mutex::scoped_lock lock(mMutex);
        return mJobs.push(inJob);
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


} // namespace Tetris
