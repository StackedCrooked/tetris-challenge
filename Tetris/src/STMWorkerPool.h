#ifndef WORKERPOOL_H
#define WORKERPOOL_H


#include "Futile/Array.h"
#include "Tetris/../../src/STMWorker.h"
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>


namespace Futile {
namespace STM {


/**
 * WorkerPool manages a pool of Worker objects.
 */
class STMWorkerPool : boost::noncopyable
{
public:
    STMWorkerPool(const std::string & inName, std::size_t inSize);

    ~STMWorkerPool();

    const std::string & name() const { return mName; }

    void schedule(const STMWorker::Task & inTask);

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
    void interruptRange(std::size_t inBegin, std::size_t inCount);

    std::string mName;
    std::size_t mRotation;

    typedef boost::shared_ptr<STMWorker> WorkerPtr;
    typedef std::vector<WorkerPtr> Workers;
    Workers mWorkers;

    mutable boost::mutex mMutex;
};


} } // namespace Futile::STM


#endif // WORKERPOOL_H
