#ifndef TETRIS_THREADINGCONFIGURATIONBOOST_H
#define TETRIS_THREADINGCONFIGURATIONBOOST_H


#include <boost/thread.hpp>


namespace Tetris {


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


} // namespace Tetris


#endif // TETRIS_THREADINGCONFIGURATIONBOOST_H
