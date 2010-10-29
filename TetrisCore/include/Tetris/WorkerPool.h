#ifndef TETRIS_WORKERPOOL_H_INCLUDED
#define TETRIS_WORKERPOOL_H_INCLUDED


#include "Worker.h"
#include <vector>
#include <boost/shared_ptr.hpp>


namespace Tetris
{

    /**
     * WorkerPool manages a pool of WorkerThreads
     */
    class WorkerPool
    {
    public:
        WorkerPool(const std::string & inName, size_t inSize);

        ~WorkerPool();

        const std::string & name() const { return mName; }

        void schedule(const Worker::Task & inTask);

        // Returns the number of workers.
        size_t size() const;

        // Wait until all Workers have finished their queue
        void wait();
        
        // Interrupts all workers.
        void interruptAndClearQueue();
        
        struct Stats
        {
            Stats(int inActiveWorkerCount, int inTotalWorkerCount) :
                activeWorkerCount(inActiveWorkerCount),
                totalWorkerCount(inTotalWorkerCount)
            {
            }

            int activeWorkerCount;
            int totalWorkerCount;
        };

        Stats stats() const;

    private:
        // non-coyable
        WorkerPool(const WorkerPool&);
        WorkerPool& operator=(const WorkerPool&);

        void interruptRange(size_t inBegin, size_t inCount);

        std::string mName;
        typedef std::vector<WorkerPtr> Workers;
        mutable size_t mRotation;

        Workers mWorkers;
        mutable boost::mutex mMutex;
    };

} // namespace Tetris


#endif // TETRIS_WORKERPOOL_H_INCLUDED
