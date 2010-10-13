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
    class WorkerPool : boost::noncopyable
    {
    public:
        WorkerPool(const std::string & inName, size_t inSize);

        ~WorkerPool();

        inline const std::string & name() const { return mName; }

        // Returns a different worker on each call.
        // This enables you to evenly spread tasks.
        WorkerPtr getWorker();

        WorkerPtr getWorker(size_t inIndex);

        // Returns the number of workers.
        size_t size() const;

        // Change the number of workers in the pool.
        // Setting a smaller size when there are workers active is safe, but interrupts their task.
        void setSize(size_t inSize);
        
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
        void interruptRange(size_t inBegin, size_t inCount);

        std::string mName;
        typedef std::vector<WorkerPtr> Workers;
        mutable size_t mRotation;

        Workers mWorkers;
        mutable boost::mutex mWorkersMutex;
    };

} // namespace Tetris


#endif // TETRIS_WORKERPOOL_H_INCLUDED
