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
        // Creates a new thread and starts waiting for tasks.
        WorkerThread();

        // If a task is running during destruction then we send
        // it an interrupt signal and wait for it to return.
        // Any remaining tasks are erased and the thread is destroyed.
        ~WorkerThread();

        typedef boost::function<void()> Task;

        // The task should be interruptable. This can be achieved by peridically
        // calling the 'boost::this_thread::interruption_point()' function.
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
