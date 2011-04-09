#ifndef FUTILE_WORKERPOOL_H_INCLUDED
#define FUTILE_WORKERPOOL_H_INCLUDED


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
    WorkerPool(const std::string & inName, size_t inSize);

    ~WorkerPool();

    const std::string & name() const { return mName; }

    void schedule(const Worker::Task & inTask);

    // Returns the number of workers.
    size_t size() const;

    int getActiveWorkerCount() const;

    // Change the number of workers in the pool.
    // Setting a smaller size when there are workers active is safe, but interrupts their task.
    void resize(size_t inSize);

    // Wait until all Workers have finished their queue
    void wait();

    // Interrupts all workers.
    void interruptAndClearQueue();

private:
    // non-coyable
    WorkerPool(const WorkerPool&);
    WorkerPool& operator=(const WorkerPool&);

    void interruptRange(size_t inBegin, size_t inCount);

    std::string mName;
    size_t mRotation;

    typedef boost::shared_ptr<Worker> WorkerPtr;
    typedef std::vector<WorkerPtr> Workers;
    Workers mWorkers;

    mutable Mutex mMutex;
};


} // namespace Futile


#endif // FUTILE_WORKERPOOL_H_INCLUDED