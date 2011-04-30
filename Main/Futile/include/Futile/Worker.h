#ifndef FUTILE_WORKERTHREAD_H_INCLUDED
#define FUTILE_WORKERTHREAD_H_INCLUDED


#include "Futile/Threading.h"
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#include <list>


namespace Futile {


class Worker;
class WorkerPool;


/**
 * Worker starts a background thread that runs in a loop processing tasks from a queue.
 */
class Worker
{
public:
    // Creates a new thread and starts waiting for tasks.
    Worker(const std::string & inName);

    // If a task is running during destruction then we send
    // it an interrupt signal and wait for it to return.
    // Any remaining tasks are erased and the thread is destroyed.
    ~Worker();

    inline const std::string & name() const { return mName; }

    typedef boost::function<void()> Task;

    // Adds a task to the queue.
    // The task should be interruptable. This can be achieved by letting it
    // periodically call the boost::this_thread::interruption_point() function.
    void schedule(const Task & inTask);

    // Returns the number of pending tasks.
    size_t size() const;

    enum Status
    {
        Status_Initial,
        Status_Waiting,
        Status_Scheduled,
        Status_Working,
        Status_FinishedOne
    };

    /**
     * Get the current status.
     */
    Status status() const;

    // Wait for the Worker to finish all tasks.
    void wait();

    void waitForStatus(Status inStatus);

    enum InterruptOption
    {
        InterruptOption_Wait,
        InterruptOption_DontWait
    };

    /**
     * Sends an interrupt message to the current task. This method is
     * blocking until the task has completed. To improve responsiveness
     * the functor should periodically call
     * boost::this_thread::interruption_point().
     *
     * After this method has returned the worker starts on the next task
     * or enters the waiting state.
     */
    void interrupt(InterruptOption inInterruptOption = InterruptOption_Wait);

    /**
     * Same as interrupt() but also clears the queue.
     */
    void interruptAndClearQueue(InterruptOption inInterruptOption = InterruptOption_Wait);

private:
    Worker(const Worker&);
    Worker& operator=(const Worker&);

    friend class WorkerPool;
    void setQuitFlag();
    bool getQuitFlag() const;

    void setStatus(Status inStatus);

    void run();
    Task nextTask();
    void processTask();

    std::string mName;
    Status mStatus;
    mutable Mutex mStatusMutex;
    Condition mStatusCondition;

    std::list<Task> mQueue;
    mutable Mutex mQueueMutex;
    Condition mQueueCondition;

    mutable Mutex mQuitFlagMutex;
    bool mQuitFlag;

    boost::scoped_ptr<boost::thread> mThread;
};


std::string ToString(Worker::Status);


} // namespace Futile


#endif // FUTILE_WORKERTHREAD_H_INCLUDED