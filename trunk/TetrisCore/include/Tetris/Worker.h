#ifndef TETRIS_WORKERTHREAD_H_INCLUDED
#define TETRIS_WORKERTHREAD_H_INCLUDED


#include <list>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/thread.hpp>


namespace Tetris
{

    class Worker;
    typedef boost::shared_ptr<Worker> WorkerPtr;
    class WorkerPool;
    

    /**
     * Worker starts a background thread that runs in a loop processing tasks from a queue.
     * When the queue is empty the Worker goes into waiting mode.
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

        /**
         * Sends an interrupt message to the current task. This method is
         * blocking until the task has completed. To improve responsiveness
         * the functor should periodically call
         * boost::this_thread::interruption_point().
         *
         * After this method has returned the worker starts on the next task
         * or enters the waiting state.
         */
        void interrupt();

        /**
         * Same as interrupt() but also clears the queue.
         */
        void interruptAndClearQueue();

    private:

        friend class WorkerPool;
        void setQuitFlag();
        bool getQuitFlag() const;

        void setStatus(Status inStatus);

        void run();
        Task nextTask();
        void processTask();

        std::string mName;
        Status mStatus;
        mutable boost::mutex mStatusMutex;
        mutable boost::condition_variable mStatusCondition;

        std::list<Task> mQueue;
        mutable boost::mutex mQueueMutex;
        boost::condition_variable mQueueCondition;

        mutable boost::mutex mQuitFlagMutex;
        bool mQuitFlag;

        boost::scoped_ptr<boost::thread> mThread;
    };

    std::string ToString(Worker::Status);

} // namespace Tetris


#endif // WORKERTHREAD_H_INCLUDED
