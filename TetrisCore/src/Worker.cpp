#include "Tetris/Worker.h"
#include "Tetris/Assert.h"
#include "Tetris/Logger.h"
#include "Tetris/MakeString.h"

//
// SetThreadName
//
// Enables you to see the thread name the Visual Studio debugger.
//
#ifdef _WIN32
#include <windows.h>
    void SetThreadName(DWORD inThreadId, const std::string & inThreadName)
    {
        #pragma pack(push,8)
        typedef struct tagTHREADNAME_INFO
        {
            DWORD dwType; // Must be 0x1000.
            LPCSTR szName; // Pointer to name (in user addr space).
            DWORD inThreadId; // Thread ID (-1=caller thread).
            DWORD dwFlags; // Reserved for future use, must be zero.
        } THREADNAME_INFO;
        #pragma pack(pop)

        THREADNAME_INFO info;
        info.dwType = 0x1000;
        info.szName = inThreadName.c_str();
        info.inThreadId = inThreadId;
        info.dwFlags = 0;

        __try
        {
            const DWORD MS_VC_EXCEPTION=0x406D1388;
            RaiseException( MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info);
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
        }
    }
#else
    #define SetThreadName(...)
#endif


namespace Tetris
{
    
    std::string ToString(Worker::Status inStatus)
    {
        switch (inStatus)
        {
            case Worker::Status_Waiting:
            {
                return "Waiting";
            }
            case Worker::Status_Working:
            {
                return "Working";
            }
            default:
            {
                throw std::logic_error("Invalid enum value for Worker::Status.");
            }
        }
    }


    Worker::Worker(const std::string & inName) :
        mName(inName),
        mStatus(Status_Waiting),
        mQuitFlag(false)
    {
        mThread.reset(new boost::thread(boost::bind(&Worker::run, this)));
    }


    Worker::~Worker()
    {
        {
            setQuitFlag();
            boost::mutex::scoped_lock queueLock(mQueueMutex);
            boost::mutex::scoped_lock statusLock(mStatusMutex);
            mQueue.clear();
            mThread->interrupt();
            mQueueCondition.notify_all();
            mStatusCondition.notify_all();
        }
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


    void Worker::setStatus(Status inStatus)
    {
        boost::mutex::scoped_lock lock(mStatusMutex);
        mStatus = inStatus;
        mStatusCondition.notify_all();
    }


    Worker::Status Worker::status() const
    {
        boost::mutex::scoped_lock lock(mStatusMutex);
        return mStatus;
    }
    
    
    void Worker::waitForStatus(Status inStatus)
    {
        boost::mutex::scoped_lock lock(mStatusMutex);
        while (mStatus != inStatus)
        {
            mStatusCondition.wait(lock);
        }
    }


    size_t Worker::size() const
    {
        boost::mutex::scoped_lock lock(mQueueMutex);
        return mQueue.size();
    }


    void Worker::interrupt(bool inWaitForStatus)
    {
        boost::mutex::scoped_lock statusLock(mStatusMutex);
        mThread->interrupt();
        mStatusCondition.wait(statusLock);
    }


    void Worker::interruptAndClearQueue(bool inWaitForStatus)
    {
        boost::scoped_ptr<boost::mutex::scoped_lock> queueLock(new boost::mutex::scoped_lock(mQueueMutex));
        mQueue.clear();
        boost::mutex::scoped_lock statusLock(mStatusMutex);
        mThread->interrupt();
        mQueueCondition.notify_all();
        queueLock.reset();
        mStatusCondition.wait(statusLock);
    }

    void Worker::schedule(const Worker::Task & inTask)
    {
        boost::mutex::scoped_lock lock(mQueueMutex);
        mQueue.push_back(inTask);
        mQueueCondition.notify_all();
    }


    Worker::Task Worker::nextTask()
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
        setStatus(Status_Waiting);
    }


    void Worker::run()
    {        
        // Wrap entire thread in try/catch block.
        try
        {
            SetThreadName(::GetCurrentThreadId(), mName);
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
