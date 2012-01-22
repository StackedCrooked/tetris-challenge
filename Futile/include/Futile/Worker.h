#ifndef WORKERTHREAD_H
#define WORKERTHREAD_H


#include "Futile/Atomic.h"
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <list>


namespace Futile {


class Worker;
class WorkerPool;


/**
 * WorkerStatus enum
 *
 * Utility functions:
 *  - EnumInfo<WorkerStatus>::ToString(inWorkerStatus);
 *   -EnumInfo<WorkerStatus>::FromString(inWorkerStatus);
 */
enum WorkerStatus
{
    WorkerStatus_Idle,
    WorkerStatus_Scheduled,
    WorkerStatus_Working,
    WorkerStatus_FinishedOne
};


/**
 * Worker starts a background thread that runs in a loop processing tasks from a queue.
 */
class Worker : boost::noncopyable
{
public:
    // Creates a new thread and starts waiting for tasks.
    Worker(const std::string & inName);

    // If a task is running during destruction then we send
    // it an interrupt signal and wait for it to return.
    // Any remaining tasks are erased and the thread is destroyed.
    ~Worker();

    const std::string & name() const { return mName; }

    typedef boost::function<void()> Task;

    // Adds a task to the queue.
    // The task should be interruptable. This can be achieved by letting it
    // periodically call the boost::this_thread::interruption_point() function.
    void schedule(const Task & inTask);

    // Returns the number of pending tasks.
    std::size_t size() const;

    // No tasks queued.
    bool empty() const;

    /**
     * Get the current status.
     */
    WorkerStatus status() const;

    // Wait for the Worker to finish all tasks.
    void wait();

    // Wait for a certain status.
    void waitForStatus(WorkerStatus inStatus);

    /**
     * Sends an interrupt message to the current task. This method is
     * blocking until the task has completed. To improve responsiveness
     * the functor should periodically call
     * boost::this_thread::interruption_point().
     *
     * After this method has returned the worker starts on the next task
     * or enters the waiting state.
     *
     * @param    inJoin    If "true" then this call blocks until the Worker has finished.
     *                     If "false" then this call returns immediately.
     */
    void interrupt(bool inJoin = true);

    /**
     * Same as interrupt() but also clears the queue.
     * @param    inJoin    If "true" then this call waits until the Worker has finished and the queue has been cleared.
     *                     If "false" then this call returns immediately.
     */
    void interruptAndClearQueue(bool inJoin = true);

private:
    friend class WorkerPool;

    void setStatus(WorkerStatus inStatus);

    void run();
    Task nextTask();
    void processTask();

    std::string mName;
    WorkerStatus mStatus;
    mutable boost::mutex mStatusMutex;
    boost::condition_variable mStatusCondition;

    std::list<Task> mQueue;
    mutable boost::mutex mQueueMutex;
    boost::condition_variable mQueueCondition;

    Atomic<bool> mQuitFlag;

    boost::scoped_ptr<boost::thread> mThread;
};


} // namespace Futile


#endif // WORKERTHREAD_H
