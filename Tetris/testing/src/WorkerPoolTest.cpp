#include "WorkerPoolTest.h"
#include "Futile/Threading.h"
#include "Futile/WorkerPool.h"
#include "Futile/Assert.h"
#include "Poco/Thread.h"
#include <iostream>


namespace testing {
namespace { // anonymous


const int cSleepTime = 100; // ms
const int cMargin    = 200; // ms
const std::size_t cPoolSize[] = {1, 2, 4, 8, 16, 32};
const std::size_t cPoolSizeCount = sizeof(cPoolSize) / sizeof(cPoolSize[0]);


} // anonymous namespace


WorkerPoolTest::WorkerPoolTest() :
    TetrisTest(),
    mIterationCount(10)
{
}


TEST_F(WorkerPoolTest, Basic)
{
    // Test without interrupt
    for (std::size_t i = 0; i < cPoolSizeCount; ++i)
    {
        mStopwatch.restart();
        WorkerPool pool("WorkerPool Test", cPoolSize[i]);
        for (size_t idx = 0; idx != cPoolSize[i]; ++idx)
        {
            pool.schedule(boost::bind(&Poco::Thread::sleep, cSleepTime));
        }
        pool.wait();
        mStopwatch.stop();
        int elapsedMs = static_cast<int>(mStopwatch.elapsed() / 1000);
        Assert(elapsedMs - cSleepTime > -cMargin);
        Assert(elapsedMs - cSleepTime < cMargin);
    }
}



TEST_F(WorkerPoolTest, Interrupt)
{
    // Test with interrupt
    for (size_t i = 0; i < cPoolSizeCount; ++i)
    {
        mStopwatch.restart();
        WorkerPool pool("WorkerPool Test", cPoolSize[i]);
        for (size_t idx = 0; idx != cPoolSize[i]; ++idx)
        {
            pool.schedule(boost::bind(&WorkerPoolTest::BeBusy));
        }
        pool.interruptAndClearQueue();
        int elapsedMs = static_cast<int>(mStopwatch.elapsed() / 1000);
        Assert(elapsedMs - cSleepTime > -cMargin);
        Assert(elapsedMs - cSleepTime < cMargin);
    }
}


TEST_F(WorkerPoolTest, Resize)
{
    // Test resize
    for (std::size_t i = 0; i < cPoolSizeCount; ++i)
    {
        mStopwatch.restart();
        WorkerPool pool("WorkerPool Test", cPoolSize[i]);
        for (size_t idx = 0; idx != cPoolSize[i]; ++idx)
        {
            pool.schedule(boost::bind(&WorkerPoolTest::BeBusy));
        }

        pool.resize(cPoolSize[i]);
        Assert(pool.size() == cPoolSize[i]);

        std::size_t newSize = static_cast<int>(0.5 + (cPoolSize[i] / 2.0));
        pool.resize(newSize);
        Assert(pool.size() == newSize);

        pool.resize(0);
        Assert(pool.size() == 0);

        mStopwatch.stop();
        int elapsedMs = static_cast<int>(mStopwatch.elapsed() / 1000);
        Assert(elapsedMs - cSleepTime > -cMargin);
        Assert(elapsedMs - cSleepTime < cMargin);
    }
}


TEST_F(WorkerPoolTest, JoinAll)
{
    // Test joinAll
    for (std::size_t i = 0; i < cPoolSizeCount; ++i)
    {
        mStopwatch.restart();
        WorkerPool pool("WorkerPool Test", cPoolSize[i]);
        for (size_t i = 0; i < pool.size(); ++i)
        {
            pool.schedule(boost::bind(&Poco::Thread::sleep, 250));
        }
        pool.wait();
        mStopwatch.stop();
    }
}


} // namespace testing
