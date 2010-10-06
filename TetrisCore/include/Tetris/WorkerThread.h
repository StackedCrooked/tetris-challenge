#ifndef WORKERTHREAD_H_INCLUDED
#define WORKERTHREAD_H_INCLUDED


#include <list>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/thread.hpp>


namespace Tetris
{

    /**
     * WorkerThread is a thread that runs in a loop taking "work" from a queue.
     * When the queue is empty the thread waits.
     */
    class WorkerThread : boost::noncopyable
    {
    public:
        // Creates a new thread and starts waiting for tasks.
        WorkerThread();

        // If a task is running during destruction then we send
        // it an interrupt signal and wait for it to return.
        // Any remaining tasks are erased and the thread is destroyed.
        ~WorkerThread();

        typedef boost::function<void()> Task;

        // Adds a task to the queue.
        // The task should be interruptable. This can be achieved by letting it
        // periodically call the boost::this_thread::interruption_point() function.
        void schedule(const Task & inTask);

        // Returns the number of pending tasks.
        size_t queueSize() const;

        enum Status
        {
            Status_Nil,
            Status_Waiting,
            Status_Working
        };

        // Get the current status.
        Status status() const;

        // Erases all pending tasks. Does not affect the currently active task.
        void clear();

        // Sends an interrupt message to the current task.
        // This call is blocking until the task has completed.
        // After that the WorkerThread will start working on
        // the next task or enter waiting mode.
        void interrupt();

        // Combines the clear() interrupt() calls.
        // Blocks until task returns.
        void clearAndInterrupt();

    private:
        void setQuitFlag();
        bool getQuitFlag() const;

        void setStatus(Status inStatus);

        void run();
        Task nextTask();
        void processTask();
        
        Status mStatus;
        mutable boost::mutex mStatusMutex;

        std::list<Task> mQueue;
        mutable boost::mutex mQueueMutex;
        boost::condition_variable mQueueCondition;

        boost::condition_variable mTaskProcessedCondition;

        mutable boost::mutex mQuitFlagMutex;
        bool mQuitFlag;

        boost::scoped_ptr<boost::thread> mThread;
    };

} // namespace Tetris


#endif // WORKERTHREAD_H_INCLUDED
