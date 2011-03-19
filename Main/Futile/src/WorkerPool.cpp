#include "Futile/Config.h"
#include "Futile/WorkerPool.h"
#include "Futile/Logging.h"
#include "Futile/MakeString.h"
#include "Futile/Threading.h"
#include <boost/noncopyable.hpp>


namespace Futile {


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
    ScopedLock lock(mMutex);
    mRotation = (mRotation + 1) % mWorkers.size();
    mWorkers[mRotation]->schedule(inTask);
}


size_t WorkerPool::size() const
{
    ScopedLock lock(mMutex);
    return mWorkers.size();
}


void WorkerPool::resize(size_t inSize)
{
    ScopedLock lock(mMutex);
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
    ScopedLock lock(mMutex);
    for (size_t idx = 0; idx != mWorkers.size(); ++idx)
    {
        Worker & worker = *mWorkers[idx];
        worker.wait();
    }
}


void WorkerPool::interruptRange(size_t inBegin, size_t inCount)
{
    LockMany<Mutex> locker;

    //
    // Lock all workers' queue and status.
    //
    for (size_t idx = inBegin; idx != inBegin + inCount; ++idx)
    {
        Worker & worker = *mWorkers[idx];
        locker.lock(worker.mQueueMutex);
        worker.mQueue.clear();
    }

    //
    // Interrupt all threads.
    //
    for (size_t idx = inBegin; idx != inBegin + inCount; ++idx)
    {
        Worker & worker = *mWorkers[idx];
        worker.interrupt(Worker::InterruptOption_DontWait);
    }

    //
    // Wait until all workers are ready.
    //
    for (size_t idx = inBegin; idx != inBegin + inCount; ++idx)
    {
        Worker & worker = *mWorkers[idx];
        ScopedLock statusLock(worker.mStatusMutex);
        if (worker.mStatus == Worker::Status_Working)
        {
            worker.mStatusCondition.wait(statusLock);
        }
    }
}


void WorkerPool::interruptAndClearQueue()
{
    ScopedLock lock(mMutex);
    interruptRange(0, mWorkers.size());
}


int WorkerPool::getActiveWorkerCount() const
{
    ScopedLock lock(mMutex);
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


} // namespace Futile
