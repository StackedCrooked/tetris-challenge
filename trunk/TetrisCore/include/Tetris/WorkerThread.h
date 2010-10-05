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

        ~WorkerThread();

        typedef boost::function<void()> Task;

        void schedule(const Task & inTask);

        // Clears the task queue.
        void clear();

        // Sends an interrupt message to the current thread and then joins the thread.
        // This call effectively destroys the current thread.
        void interrupt();

        // After an interrupt this method can be called to recreate
        // the thread and resume the processing of the task list.
        void resume();

    private:
        void runImpl();

        std::list<Task> mQueue;
        boost::mutex mQueueMutex;
        boost::condition_variable mQueueCondition;
        boost::scoped_ptr<boost::thread> mThread;
    };

} // namespace Tetris


#endif // WORKERTHREAD_H_INCLUDED
