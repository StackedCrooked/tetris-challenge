#ifndef FUTILE_THREADINGCONFIGURATIONBOOST_H
#define FUTILE_THREADINGCONFIGURATIONBOOST_H


#include <boost/thread.hpp>


namespace Futile {


typedef boost::mutex Mutex;
typedef boost::mutex::scoped_lock ScopedLock;
typedef boost::shared_mutex SharedMutex;
typedef boost::upgrade_lock<SharedMutex> SharedLock;
typedef boost::upgrade_to_unique_lock<SharedMutex> UniqueLock;
typedef boost::condition_variable ConditionVariable;


inline void LockMutex(Mutex & inMutex)
{
    inMutex.lock();
}


inline void UnlockMutex(Mutex & inMutex)
{
    inMutex.unlock();
}


} // namespace Futile


#endif // FUTILE_THREADINGCONFIGURATIONBOOST_H
