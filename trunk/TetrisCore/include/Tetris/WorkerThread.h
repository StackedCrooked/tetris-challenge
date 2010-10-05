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
     * When the queue is empty it waits.
     */
    class WorkerThread : boost::noncopyable
    {
    public:
        WorkerThread();

        // If a task is running while destructing an interrupt
        // is sent to the current task and then we wait for it
        // to return.
        ~WorkerThread();

        typedef boost::function<void()> Task;

        // In order to be interruptible the task function should 
        // periodically call boost::this_thread::interruption_point().
        void schedule(const Task & inTask);

        // Sends an interrupt message to the current task and waits for it to return.
        void interrupt();

        // Erases all pending tasks. Does not affect the currently active task.
        void clear();

    private:
        void setQuitFlag();
        bool getQuitFlag() const;

        void run();
        Task nextTask();
        void processTask();

        boost::scoped_ptr<boost::thread> mThread;
        std::list<Task> mQueue;
        boost::mutex mQueueMutex;
        boost::condition_variable mQueueCondition;

        boost::mutex mTaskProcessedMutex;
        boost::condition_variable mTaskProcessedCondition;
        mutable boost::mutex mQuitFlagMutex;
        bool mQuitFlag;
    };

} // namespace Tetris


#endif // WORKERTHREAD_H_INCLUDED
