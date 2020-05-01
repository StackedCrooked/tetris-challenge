#ifndef WORKERPOOL_H_INCLUDED
#define WORKERPOOL_H_INCLUDED


#include "Futile/Array.h"
#include "Futile/Worker.h"
#include <vector>
#include <boost/shared_ptr.hpp>


namespace Futile {


/**
 * WorkerPool manages a pool of Worker objects.
 */
class WorkerPool
{
public:
    WorkerPool(const std::string& inName, std::size_t inSize);

    ~WorkerPool();

    const std::string& name() const { return mName; }

    void schedule(const Worker::Task& inTask);

    // Returns the number of workers.
    std::size_t size() const;

    int getActiveWorkerCount() const;

    // Change the number of workers in the pool.
    // Setting a smaller size when there are workers active is safe, but interrupts their task.
    void resize(std::size_t inSize);

    // Wait until all Workers have finished their queue
    void wait();

    // Interrupts all workers.
    // This call is blocking until all workers have stopped.
    void interruptAndClearQueue();

private:
    // non-coyable
    WorkerPool(const WorkerPool&);
    WorkerPool& operator=(const WorkerPool&);

    void interruptRange(std::size_t inBegin, std::size_t inCount);

    std::string mName;
    std::size_t mRotation;

    typedef boost::shared_ptr<Worker> WorkerPtr;
    typedef std::vector<WorkerPtr> Workers;
    Workers mWorkers;

    mutable Mutex mMutex;
};


} // namespace Futile


#endif // WORKERPOOL_H_INCLUDED
