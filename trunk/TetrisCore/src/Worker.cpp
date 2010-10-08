#include "Tetris/Worker.h"
#include "Tetris/Logger.h"
#include "Tetris/MakeString.h"


namespace Tetris
{

    Worker::Worker() :
        mStatus(Status_Nil),
        mQuitFlag(false)

    {
        mThread.reset(new boost::thread(boost::bind(&Worker::run, this)));
    }


    Worker::~Worker()
    {
        setQuitFlag();
        interrupt();
        mThread->join();
        mThread.reset();
    }


    void Worker::setQuitFlag()
    {
        boost::mutex::scoped_lock lock(mQuitFlagMutex);
        mQuitFlag = true;
    }


    bool Worker::getQuitFlag() const
    {
        boost::mutex::scoped_lock lock(mQuitFlagMutex);
        return mQuitFlag;
    }


    Worker::Status Worker::status() const
    {
        boost::mutex::scoped_lock lock(mStatusMutex);
        return mStatus;
    }


    void Worker::setStatus(Status inStatus)
    {
        boost::mutex::scoped_lock lock(mStatusMutex);
        mStatus = inStatus;
    }


    size_t Worker::queueSize() const
    {
        boost::mutex::scoped_lock lock(mQueueMutex);
        return mQueue.size();
    }


    void Worker::clear()
    {
        boost::mutex::scoped_lock lock(mQueueMutex);
        mQueue.clear();
    }


    void Worker::interrupt()
    {
        boost::mutex::scoped_lock queueLock(mQueueMutex);

        // Interrupt the current task.
        mThread->interrupt();

        // If no task is currently being processed then
        // we'll need to break out of the wait() call.
        mQueueCondition.notify_all();

        // This blocks the thread until the notify_all() call in processTask().
        mTaskProcessedCondition.wait(queueLock);
    }


    void Worker::clearAndInterrupt()
    {
        boost::mutex::scoped_lock queueLock(mQueueMutex);

        // Clear the queue
        mQueue.clear();

        // Interrupt the current task.
        mThread->interrupt();

        // If no task is currently being processed then
        // we'll need to break out of the wait() call.
        mQueueCondition.notify_all();

        // This blocks the thread until the notify_all() call in processTask().
        mTaskProcessedCondition.wait(queueLock);
    }


    void Worker::schedule(const Worker::Task & inTask)
    {
        boost::mutex::scoped_lock lock(mQueueMutex);
        mQueue.push_back(inTask);
        mQueueCondition.notify_one();
    }


    Worker::Task Worker::nextTask()
    {
        boost::mutex::scoped_lock lock(mQueueMutex);
        while (mQueue.empty())
        {
            setStatus(Status_Waiting);
            mQueueCondition.wait(lock);
            boost::this_thread::interruption_point();
        }
        Task task = mQueue.front();
        mQueue.erase(mQueue.begin());
        return task;
    }


    void Worker::processTask()
    {
        try
        {
            // Get the next task.
            Task task = nextTask();

            // Run the task.
            setStatus(Status_Working);
            task();
        }
        catch (const boost::thread_interrupted &)
        {
            // Task was interrupted. Ok.
        }
        mTaskProcessedCondition.notify_all();
    }


    void Worker::run()
    {
        // Wrap entire thread in try/catch block.
        try
        {
            while (!getQuitFlag())
            {
                processTask();
            }
        }
        catch (const std::exception & inExc)
        {
            LogError(MakeString() << "Exception caught in Worker::run. Detail: " << inExc.what());
        }
        catch (...)
        {
            LogError("Unknown exception caught in Worker::run.");
        }
    }

} // namespace Tetris
