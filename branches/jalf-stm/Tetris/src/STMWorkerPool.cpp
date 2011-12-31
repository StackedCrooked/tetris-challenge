#include "Tetris/../../src/STMWorkerPool.h"
#include "Futile/Logging.h"
#include "Futile/MakeString.h"
#include <boost/noncopyable.hpp>


namespace Futile {
namespace STM {


STMWorkerPool::STMWorkerPool(const std::string & inName, std::size_t inSize) :
    mName(inName),
    mRotation(0),
    mWorkers(),
    mMutex()
{
    for (std::size_t idx = 0; idx != inSize; ++idx)
    {
        WorkerPtr workerPtr(new STMWorker(MakeString() << inName << mWorkers.size()));
        mWorkers.push_back(workerPtr);
    }
}


STMWorkerPool::~STMWorkerPool()
{
    interruptAndClearQueue();
}


void STMWorkerPool::schedule(const STMWorker::Task & inTask)
{
    boost::mutex::scoped_lock lock(mMutex);
    mRotation = (mRotation + 1) % mWorkers.size();
    mWorkers[mRotation]->schedule(inTask);
}


std::size_t STMWorkerPool::size() const
{
    boost::mutex::scoped_lock lock(mMutex);
    return mWorkers.size();
}


void STMWorkerPool::resize(std::size_t inSize)
{
    boost::mutex::scoped_lock lock(mMutex);
    if (inSize > mWorkers.size())
    {
        while (mWorkers.size() < inSize)
        {
            WorkerPtr workerPtr(new STMWorker(MakeString() << mName << mWorkers.size()));
            mWorkers.push_back(workerPtr);
        }
    }
    else if (inSize < mWorkers.size()) // Deletes a few workers
    {
        interruptRange(inSize, mWorkers.size() - inSize);
        mWorkers.resize(inSize);
    }
}


void STMWorkerPool::wait()
{
    boost::mutex::scoped_lock lock(mMutex);
    for (std::size_t idx = 0; idx != mWorkers.size(); ++idx)
    {
        STMWorker & worker = *mWorkers[idx];
        worker.wait();
    }
}


void STMWorkerPool::interruptRange(std::size_t inBegin, std::size_t inCount)
{
    LockMany<boost::mutex> locker;

    //
    // Lock all workers.
    //
    for (std::size_t idx = inBegin; idx != inBegin + inCount; ++idx)
    {
        STMWorker & worker = *mWorkers[idx];
        locker.lock(worker.mQueueMutex);

        // Keep queue locked for now.
    }

    //
    // Clear queues and interrupt the work.
    //
    for (std::size_t idx = inBegin; idx != inBegin + inCount; ++idx)
    {
        STMWorker & worker = *mWorkers[idx];

        worker.mQueue.clear();
        worker.interrupt(false);

        // NOTE: don't wait here, because that would be wasteful.
    }

    //
    // Wait until all workers are ready.
    //
    for (std::size_t idx = inBegin; idx != inBegin + inCount; ++idx)
    {
        STMWorker & worker = *mWorkers[idx];
        boost::mutex::scoped_lock statusLock(worker.mStatusMutex);
        if (worker.mStatus == WorkerStatus_Working)
        {
            worker.mStatusCondition.wait(statusLock);
        }
    }
}


void STMWorkerPool::interruptAndClearQueue()
{
    boost::mutex::scoped_lock lock(mMutex);
    interruptRange(0, mWorkers.size());
}


int STMWorkerPool::getActiveWorkerCount() const
{
    boost::mutex::scoped_lock lock(mMutex);
    int activeWorkerCount = 0;
    for (std::size_t idx = 0; idx != mWorkers.size(); ++idx)
    {
        const STMWorker & worker = *mWorkers[idx];
        if (worker.status() == WorkerStatus_Working)
        {
            activeWorkerCount++;
        }
    }
    return activeWorkerCount;
}


} } // namespace Futile::STM
