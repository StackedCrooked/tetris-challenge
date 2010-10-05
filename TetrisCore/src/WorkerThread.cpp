#include "Tetris/WorkerThread.h"
#include "Tetris/Logger.h"


namespace Tetris
{

    WorkerThread::WorkerThread() :
        mThread(),
        mQuitFlag(false)
    {
        mThread.reset(new boost::thread(boost::bind(&WorkerThread::run, this)));
    }


    WorkerThread::~WorkerThread()
    {
        setQuitFlag();
        mThread->interrupt();
        mQueueCondition.notify_one();
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

    
    void WorkerThread::schedule(const WorkerThread::Task & inTask)
    {
        {
            boost::mutex::scoped_lock lock(mQueueMutex);
            mQueue.push_back(inTask);
        }
        mQueueCondition.notify_one();
    }


    void WorkerThread::clear()
    {
        boost::mutex::scoped_lock lock(mQueueMutex);
        mQueue.clear();
    }


    void WorkerThread::interrupt()
    {
        boost::mutex::scoped_lock lock(mInterruptMutex);
        mThread->interrupt();
        mQueueCondition.notify_one();
        mInterruptCondition.wait(lock);
    }


    WorkerThread::Task WorkerThread::nextTask()
    {
        boost::mutex::scoped_lock lock(mQueueMutex);
        while (mQueue.empty())
        {
            mQueueCondition.wait(lock);

            // Interrupt here happens during destruction.
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

            // Check if the user requested an interrupt.
            boost::this_thread::interruption_point();
        }
        catch (const boost::thread_interrupted &)
        {
            mInterruptCondition.notify_one();
            LogInfo("Worker thread was interrupted.");
        }
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
