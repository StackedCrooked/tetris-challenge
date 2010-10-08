#include "Tetris/WorkerPool.h"


namespace Tetris
{

    WorkerPool::WorkerPool(size_t inSize) :
        mWorkers(),
        mRotation(0)
    {
        for (size_t idx = 0; idx != inSize; ++idx)
        {
            mWorkers.push_back(WorkerPtr(new Worker));            
        }
    }
    
    
    Worker * WorkerPool::getWorker(size_t inIndex)
    {
        return mWorkers[inIndex].get();
    }
    
    
    Worker * WorkerPool::getWorker()
    {        
        Worker * result = mWorkers[mRotation].get();
        mRotation = (mRotation + 1) % mWorkers.size();
        return result;
    }
    
    
    size_t WorkerPool::size() const
    {
        return mWorkers.size();
    }


    void WorkerPool::setSize(size_t inSize)
    {
        if (inSize > mWorkers.size())
        {
            mWorkers.resize(inSize, WorkerPtr(new Worker));
        }
        else if (inSize < mWorkers.size()) // Deletes a few workers
        {
            mWorkers.resize(inSize);
        }
    }
    
    
    void WorkerPool::interruptAll()
    {
        // Some typedefs to reduce the typing
        typedef boost::mutex::scoped_lock ScopedLock;
        typedef boost::shared_ptr<ScopedLock> ScopedLockPtr;
        typedef std::vector<ScopedLockPtr> ScopedLocks;

        ScopedLocks locks;
        
        // Send all workers the interrupt all without waiting for the tasks.
        for (size_t idx = 0; idx != mWorkers.size(); ++idx)
        {
            Worker & worker = *mWorkers[idx];
            locks.push_back(ScopedLockPtr(new ScopedLock(worker.mQueueMutex)));
            worker.interruptImpl();
        }

	    // Wait for the tasks to finish.
        for (size_t idx = 0; idx != mWorkers.size(); ++idx)
        {
            Worker & worker = *mWorkers[idx];
            worker.mTaskProcessedCondition.wait(*locks[idx]);
        }
    }

} // namespace Tetris
