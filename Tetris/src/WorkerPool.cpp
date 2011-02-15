#include "Tetris/Config.h"
#include "Tetris/WorkerPool.h"
#include "Tetris/Logging.h"
#include "Tetris/MakeString.h"
#include "Tetris/Threading.h"
#include <boost/noncopyable.hpp>


namespace Tetris {


WorkerPool::WorkerPool(const std::string & inName, size_t inSize) :
    mName(inName),
    mRotation(0),
    mWorkers(),
    mMutex()
{
    for (size_t idx = 0; idx != inSize; ++idx)
    {
        WorkerPtr workerPtr(new Worker(MakeString() << inName << mWorkers.size()));
        mWorkers.push_back(workerPtr);
    }
}


WorkerPool::~WorkerPool()
{
    interruptAndClearQueue();
}


void WorkerPool::schedule(const Worker::Task & inTask)
{
    boost::mutex::scoped_lock lock(mMutex);
    mRotation = (mRotation + 1) % mWorkers.size();
    mWorkers[mRotation]->schedule(inTask);
}


size_t WorkerPool::size() const
{
    boost::mutex::scoped_lock lock(mMutex);
    return mWorkers.size();
}


void WorkerPool::resize(size_t inSize)
{
    boost::mutex::scoped_lock lock(mMutex);
    if (inSize > mWorkers.size())
    {
        while (mWorkers.size() < inSize)
        {
            WorkerPtr workerPtr(new Worker(MakeString() << mName << mWorkers.size()));
            mWorkers.push_back(workerPtr);
        }
    }
    else if (inSize < mWorkers.size()) // Deletes a few workers
    {
        interruptRange(inSize, mWorkers.size() - inSize);
        mWorkers.resize(inSize);
    }
}


void WorkerPool::wait()
{
    boost::mutex::scoped_lock lock(mMutex);
    for (size_t idx = 0; idx != mWorkers.size(); ++idx)
    {
        Worker & worker = *mWorkers[idx];
        worker.wait();
    }
}


void WorkerPool::interruptRange(size_t inBegin, size_t inCount)
{
    LockMany<boost::mutex> locker;

    //
    // Lock all workers' queue and status.
    //
    for (size_t idx = inBegin; idx != inBegin + inCount; ++idx)
    {
        Worker & worker = *mWorkers[idx];
        locker.lock(worker.mQueueMutex);        worker.mQueue.clear();
    }

    //
    // Interrupt all threads.
    //
    for (size_t idx = inBegin; idx != inBegin + inCount; ++idx)
    {
        Worker & worker = *mWorkers[idx];
        worker.mThread->interrupt();
    }

    //
    // Wait until all workers are ready.
    //
    for (size_t idx = inBegin; idx != inBegin + inCount; ++idx)
    {
        Worker & worker = *mWorkers[idx];
        boost::mutex::scoped_lock statusLock(worker.mStatusMutex);
        if (worker.mStatus == Worker::Status_Working)
        {
            worker.mStatusCondition.wait(statusLock);
            Assert(worker.mStatus == Worker::Status_FinishedOne);
        }
    }
}


void WorkerPool::interruptAndClearQueue()
{
    boost::mutex::scoped_lock lock(mMutex);
    interruptRange(0, mWorkers.size());
}


int WorkerPool::getActiveWorkerCount() const
{
    boost::mutex::scoped_lock lock(mMutex);
    int activeWorkerCount = 0;
    for (size_t idx = 0; idx != mWorkers.size(); ++idx)
    {
        const Worker & worker = *mWorkers[idx];
        if (worker.status() == Worker::Status_Working)
        {
            activeWorkerCount++;
        }
    }
    return activeWorkerCount;
}


} // namespace Tetris
