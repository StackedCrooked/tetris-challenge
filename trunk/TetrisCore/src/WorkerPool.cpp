#include "Tetris/WorkerPool.h"
#include "Tetris/MakeString.h"


namespace Tetris
{

    WorkerPool::WorkerPool(const std::string & inName, size_t inSize) :
        mName(inName),
        mWorkers(),
        mRotation(0)
    {
        for (size_t idx = 0; idx != inSize; ++idx)
        {
            mWorkers.push_back(WorkerPtr(new Worker(MakeString() << inName << mWorkers.size())));
        }
    }


    WorkerPool::~WorkerPool()
    {
        interruptAndClearQueue();
    }
    
    
    WorkerPtr WorkerPool::getWorker(size_t inIndex)
    {
        boost::mutex::scoped_lock workersLock(mWorkersMutex);
        return mWorkers[inIndex];
    }
    
    
    WorkerPtr WorkerPool::getWorker()
    {        
        boost::mutex::scoped_lock workersLock(mWorkersMutex);
        WorkerPtr result = mWorkers[mRotation];
        mRotation = (mRotation + 1) % mWorkers.size();
        return result;
    }
    
    
    size_t WorkerPool::size() const
    {
        boost::mutex::scoped_lock workersLock(mWorkersMutex);
        return mWorkers.size();
    }


    void WorkerPool::setSize(size_t inSize)
    {
        boost::mutex::scoped_lock workersLock(mWorkersMutex);
        if (inSize > mWorkers.size())
        {
            while (mWorkers.size() < inSize)
            {
                mWorkers.push_back(WorkerPtr(new Worker(MakeString() << mName << mWorkers.size())));
            }
        }
        else if (inSize < mWorkers.size()) // Deletes a few workers
        {
            interruptRange(inSize, mWorkers.size() - inSize);
        }
    }
    
    
    void WorkerPool::interruptRange(size_t inBegin, size_t inCount)
    {
        std::vector<boost::shared_ptr<boost::mutex::scoped_lock> > queueLocks(mWorkers.size());
        std::vector<boost::shared_ptr<boost::mutex::scoped_lock> > statusLocks(mWorkers.size());

        //
        // Lock all workers' queue and status.
        //
        for (size_t idx = inBegin; idx != inBegin + inCount; ++idx)
        {
            Worker & worker = *mWorkers[idx];
            queueLocks[idx].reset(new boost::mutex::scoped_lock(worker.mQueueMutex));
            statusLocks[idx].reset(new boost::mutex::scoped_lock(worker.mStatusMutex));
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
        // Wait for Status_Waiting on all workers.
        //
        for (size_t idx = inBegin; idx != inBegin + inCount; ++idx)
        {
            Worker & worker = *mWorkers[idx];
            while (worker.mStatus != Worker::Status_Waiting)
            {
                worker.mStatusCondition.wait(*statusLocks[idx]);
            }
        }
    }
    
    
    void WorkerPool::interruptAndClearQueue()
    {
        boost::mutex::scoped_lock workersLock(mWorkersMutex);
        interruptRange(0, mWorkers.size());
    }
    
    
    WorkerPool::Stats WorkerPool::stats() const
    {
        boost::mutex::scoped_lock workersLock(mWorkersMutex);
        int activeWorkerCount = 0;
        for (size_t idx = 0; idx != mWorkers.size(); ++idx)
        {
            Worker & worker = *mWorkers[idx];
            if (worker.status() == Worker::Status_Working)
            {
                activeWorkerCount++;
            }
        }
        return Stats(activeWorkerCount, mWorkers.size());
    }

} // namespace Tetris
