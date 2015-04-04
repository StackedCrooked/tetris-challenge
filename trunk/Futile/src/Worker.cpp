#include "Futile/Worker.h"
#include "Futile/Logging.h"
#include "Futile/MakeString.h"


namespace Futile {


Worker::Worker(const std::string & inName) :
    mName(inName),
    mStatus(WorkerStatus_Idle),
    mQuitFlag(false)
{
    mThread.reset(new boost::thread(boost::bind(&Worker::run, this)));
}


Worker::~Worker()
{
    {
        mQuitFlag.set(true);
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
    waitForStatus(WorkerStatus_Idle);
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


bool Worker::empty() const
{
    ScopedLock lock(mQueueMutex);
    return mQueue.empty();
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
        if (mStatus <= WorkerStatus_Idle)
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
        setStatus(WorkerStatus_Idle);
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
        while (!mQuitFlag.get())
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
