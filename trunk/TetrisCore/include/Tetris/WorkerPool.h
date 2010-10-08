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

        Worker * getWorker(size_t inIndex);

        // Returns the number of workers.
        size_t size() const;
        
        // Interrupts all workers.
        void interruptAll();

    private:
        typedef boost::shared_ptr<Worker> WorkerPtr;
        typedef std::vector<WorkerPtr> Workers;
        Workers mWorkers;
    };

} // namespace Tetris


#endif // TETRIS_WORKERPOOL_H_INCLUDED
