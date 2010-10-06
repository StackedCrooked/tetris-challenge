#include "Tetris/WorkerThread.h"
#include "Tetris/Logger.h"


namespace Tetris
{

    WorkerThread::WorkerThread() :
        mStatus(Status_Nil),
        mQuitFlag(false)
        
    {
        mThread.reset(new boost::thread(boost::bind(&WorkerThread::run, this)));
    }


    WorkerThread::~WorkerThread()
    {
        setQuitFlag();
        interrupt();
        mThread->join();
        mThread.reset();
    }
    
    
    void WorkerThread::setQuitFlag()
    {
        boost::mutex::scoped_lock lock(mQuitFlagMutex);
        mQuitFlag = true;
    }
    
    
    bool WorkerThread::getQuitFlag() const
    {
        boost::mutex::scoped_lock lock(mQuitFlagMutex);
        return mQuitFlag;
    }
    
    
    WorkerThread::Status WorkerThread::status() const
    {
        boost::mutex::scoped_lock lock(mStatusMutex);
        return mStatus;
    }
    
    
    void WorkerThread::setStatus(Status inStatus)
    {
        boost::mutex::scoped_lock lock(mStatusMutex);
        mStatus = inStatus;
    }
    
    
    size_t WorkerThread::queueSize() const
    {
        boost::mutex::scoped_lock lock(mQueueMutex);
        return mQueue.size();
    }


    void WorkerThread::clear()
    {
        boost::mutex::scoped_lock lock(mQueueMutex);
        mQueue.clear();
    }


    void WorkerThread::interrupt()
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


    void WorkerThread::clearAndInterrupt()
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

    
    void WorkerThread::schedule(const WorkerThread::Task & inTask)
    {
        boost::mutex::scoped_lock lock(mQueueMutex);
        mQueue.push_back(inTask);
        mQueueCondition.notify_one();
    }


    WorkerThread::Task WorkerThread::nextTask()
    {
        boost::mutex::scoped_lock lock(mQueueMutex);
        while (mQueue.empty())
        {
            mQueueCondition.wait(lock);
            boost::this_thread::interruption_point();
        }
        Task task = mQueue.front();
        mQueue.erase(mQueue.begin());
        return task;
    }
    
    
    void WorkerThread::processTask()
    {
        try
        {
            // Get the next task.
            Task task = nextTask();

            // Run the task.
            task();
        }
        catch (const boost::thread_interrupted &)
        {
            LogInfo("WorkerThread: Task was interrupted.");
        }
        mTaskProcessedCondition.notify_all();
    }
    
    
    void WorkerThread::run()
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
            LogError(MakeString() << "Exception caught in WorkerThread::run. Detail: " << inExc.what());
        }
        catch (...)
        {
            LogError("Unknown exception caught in WorkerThread::run.");
        }
    }

} // namespace Tetris
