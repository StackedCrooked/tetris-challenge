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
        WorkerPool(size_t inSize);

        // Returns a different worker on each call.
        // This enables you to evenly spread tasks.
        Worker * getWorker();

        Worker * getWorker(size_t inIndex);

        // Returns the number of workers.
        size_t size() const;

        // Change the number of workers in the pool.
        // Setting a smaller size when there are workers active is safe, but interrupts their task.
        void setSize(size_t inSize);
        
        // Interrupts all workers.
        void interruptAll();

    private:
        typedef boost::shared_ptr<Worker> WorkerPtr;
        typedef std::vector<WorkerPtr> Workers;
        mutable size_t mRotation;
        Workers mWorkers;
    };

} // namespace Tetris


#endif // TETRIS_WORKERPOOL_H_INCLUDED
