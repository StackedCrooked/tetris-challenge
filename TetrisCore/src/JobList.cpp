#include "JobList.h"
#include "GameState.h"
#include "ErrorHandling.h"
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <iostream>


namespace Tetris
{


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


} // namespace Tetris
