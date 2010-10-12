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
            Status_Nil,
            Status_Waiting,
            Status_Working
        };

        // Get the current status.
        Status status() const;

        void waitForStatus(Status inStatus);

        /**
         * Sends an interrupt message to the current task. This call is
         * blocking until the task has completed. After that the Worker
         * will start processing the next task or enter waiting mode.
         *
         * Usage:
         *   worker.interrupt(Worker::ClearQueue(true));
         *   worker.interrupt(Worker::ClearQueue(false));
         */
        void interruptOne(bool inWaitForStatus = true);
        void interruptAll(bool inWaitForStatus = true);

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
