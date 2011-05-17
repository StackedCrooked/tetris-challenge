#include "Futile/Config.h"
#include "Futile/Worker.h"
#include "Futile/Logging.h"
#include "Futile/MakeString.h"


//
// SetThreadName
//
// Enables you to see the thread name the Visual Studio debugger.
//
#if defined(_WIN32) && !defined(__MINGW32__)
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


namespace Futile {


Worker::Worker(const std::string & inName) :
    mName(inName),
    mStatus(WorkerStatus_Initial),
    mQuitFlag(false)
{
    mThread.reset(new boost::thread(boost::bind(&Worker::run, this)));
}


Worker::~Worker()
{
    {
        setQuitFlag();
        ScopedLock queueLock(mQueueMutex);
        ScopedLock statusLock(mStatusMutex);
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
    ScopedLock lock(mQuitFlagMutex);
    mQuitFlag = true;
}


bool Worker::getQuitFlag() const
{
    ScopedLock lock(mQuitFlagMutex);
    return mQuitFlag;
}


void Worker::setStatus(WorkerStatus inStatus)
{
    ScopedLock lock(mStatusMutex);
    mStatus = inStatus;
    mStatusCondition.notify_all();
}


WorkerStatus Worker::status() const
{
    ScopedLock lock(mStatusMutex);
    return mStatus;
}


void Worker::wait()
{
    waitForStatus(WorkerStatus_Waiting);
}


void Worker::waitForStatus(WorkerStatus inStatus)
{
    ScopedLock lock(mStatusMutex);
    while (mStatus != inStatus)
    {
        mStatusCondition.wait(lock);
    }
}


std::size_t Worker::size() const
{
    ScopedLock lock(mQueueMutex);
    return mQueue.size();
}


void Worker::interrupt(bool inJoin)
{
    ScopedLock statusLock(mStatusMutex);
    if (mStatus == WorkerStatus_Working)
    {
        mThread->interrupt();
        if (inJoin)
        {
            mStatusCondition.wait(statusLock);
        }
    }
}


void Worker::interruptAndClearQueue(bool inJoin)
{
    ScopedLock queueLock(mQueueMutex);
    mQueue.clear();
    ScopedLock statusLock(mStatusMutex);
    mThread->interrupt();
    mQueueCondition.notify_all();
    queueLock.unlock();
    if (inJoin)
    {
        mStatusCondition.wait(statusLock);
    }
}


void Worker::schedule(const Worker::Task & inTask)
{
    ScopedLock lock(mQueueMutex);
    mQueue.push_back(inTask);
    {
        ScopedLock statusLock(mStatusMutex);
        if (mStatus <= WorkerStatus_Waiting)
        {
            mStatus = WorkerStatus_Scheduled;
        }
    }
    mQueueCondition.notify_all();
}


Worker::Task Worker::nextTask()
{
    ScopedLock lock(mQueueMutex);
    while (mQueue.empty())
    {
        setStatus(WorkerStatus_Waiting);
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
        setStatus(WorkerStatus_Working);
        task();
    }
    catch (const boost::thread_interrupted &)
    {
        // Task was interrupted. Ok.
    }
    setStatus(WorkerStatus_FinishedOne);
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


} // namespace Futile
