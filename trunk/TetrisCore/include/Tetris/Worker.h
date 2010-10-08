#ifndef TETRIS_WORKERTHREAD_H_INCLUDED
#define TETRIS_WORKERTHREAD_H_INCLUDED


#include <list>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/thread.hpp>


namespace Tetris
{
    class WorkerPool;
    

    /**
     * Worker starts a background thread that runs in a loop processing tasks from a queue.
     * When the queue is empty the Worker goes into waiting mode.
     */
    class Worker : boost::noncopyable
    {
    public:
        // Creates a new thread and starts waiting for tasks.
        Worker();

        // If a task is running during destruction then we send
        // it an interrupt signal and wait for it to return.
        // Any remaining tasks are erased and the thread is destroyed.
        ~Worker();

        typedef boost::function<void()> Task;

        // Adds a task to the queue.
        // The task should be interruptable. This can be achieved by letting it
        // periodically call the boost::this_thread::interruption_point() function.
        void schedule(const Task & inTask);

        // Returns the number of pending tasks.
        size_t size() const;

        enum Status
        {
            Status_Nil,
            Status_Waiting,
            Status_Working,
            Status_Interrupted
        };

        // Get the current status.
        Status status() const;

        void waitForStatus(Status inStatus);

        // Sends an interrupt message to the current task.
        // This call is blocking until the task has completed.
        // After that the Worker will start working on
        // the next task or enter waiting mode.
        void interrupt();

    private:
        friend class WorkerPool;
        void setQuitFlag();
        bool getQuitFlag() const;

        void interruptImpl();

        void setStatus(Status inStatus);

        void run();
        Task nextTask();
        void processTask();

        Status mStatus;
        mutable boost::mutex mStatusMutex;
        mutable boost::condition_variable mStatusCondition;

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
