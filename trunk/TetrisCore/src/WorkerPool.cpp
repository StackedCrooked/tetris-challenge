#include "Tetris/WorkerPool.h"


namespace Tetris
{

    WorkerPool::WorkerPool(size_t inSize) :
        mWorkers(inSize, WorkerPtr(new Worker))
    {
    }
    
    
    Worker * WorkerPool::getWorker(size_t inIndex)
    {
        return mWorkers[inIndex].get();
    }
    
    
    size_t WorkerPool::size() const
    {
        return mWorkers.size();
    }
    
    
    void WorkerPool::interruptAll()
    {
        // Interrupt all workers but without waiting for the task to complete.
        for (size_t idx = 0; idx != mWorkers.size(); ++idx)
        {
            mWorkers[idx]->interruptImpl();
        }

        // Now wait for each of the workers for their task to complete.
        for (size_t idx = 0; idx != mWorkers.size(); ++idx)
        {
            mWorkers[idx]->waitForStatus(Worker::Status_Waiting);
        }
    }

} // namespace Tetris
