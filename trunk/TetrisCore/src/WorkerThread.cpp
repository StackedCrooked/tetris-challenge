#include "Tetris/WorkerThread.h"
#include "Tetris/Logger.h"


namespace Tetris
{

    WorkerThread::WorkerThread()
    {
        mThread.reset(new boost::thread(boost::bind(&WorkerThread::runImpl, this)));
    }


    WorkerThread::~WorkerThread()
    {
        if (mThread)
        {
            mThread->interrupt();
            mQueueCondition.notify_one();
            mThread->join();
        }
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
        if (mThread)
        {
            mThread->interrupt();
            mThread->join();
            mThread.reset();
        }
    }


    void WorkerThread::resume()
    {
        if (!mThread)
        {
            mThread.reset(new boost::thread(boost::bind(&WorkerThread::runImpl, this)));
        }
    }
    
    
    void WorkerThread::runImpl()
    {
        try
        {
            Task task;
            {
                boost::mutex::scoped_lock lock(mQueueMutex);
                while (mQueue.empty())
                {
                    mQueueCondition.wait(lock);
                    boost::this_thread::interruption_point();
                }
                task = mQueue.front();
                mQueue.erase(mQueue.begin());
            }            
            task();
            boost::this_thread::interruption_point();
        }
        catch (const boost::thread_interrupted &)
        {
            LogInfo("Worker thread was interrupted.");
        }
        catch (const std::exception & inExc)
        {
            LogError(MakeString() << "Unhandled exception caught in WorkerThread::runImpl. Detail: " << inExc.what());
        }
        catch (...)
        {
            LogError("Unknown exception caught in WorkerThread::runImpl.");
        }
    }

} // namespace Tetris
