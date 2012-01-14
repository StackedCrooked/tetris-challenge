#include "WorkerPoolTest.h"
#include "Futile/Assert.h"
#include "Futile/Stopwatch.h"
#include "Futile/WorkerPool.h"
#include "Futile/Threading.h"
#include <iostream>


namespace testing {


using namespace Futile;


namespace {
const UInt64 cSleepTime = 100; // ms
const UInt64 cMargin    = 400; // ms
const std::size_t cPoolSize[] = {1, 2, 4, 8, 16, 32};
const std::size_t cPoolSizeCount = sizeof(cPoolSize) / sizeof(cPoolSize[0]);
}


TEST_F(WorkerPoolTest, Basic)
{
    // Test without interrupt
    for (std::size_t i = 0; i < cPoolSizeCount; ++i)
    {
        Futile::Stopwatch stopwatch;
        stopwatch.start();
        WorkerPool pool("WorkerPool Test", cPoolSize[i]);
        for (size_t idx = 0; idx != cPoolSize[i]; ++idx)
        {
            pool.schedule(boost::bind(&Futile::Sleep, cSleepTime));
        }
        pool.wait();
        stopwatch.stop();
        Assert(stopwatch.elapsedMs() - cSleepTime > -cMargin);
        Assert(stopwatch.elapsedMs() - cSleepTime < cMargin);
    }
}


TEST_F(WorkerPoolTest, Interrupt)
{
    // Test with interrupt
    for (size_t i = 0; i < cPoolSizeCount; ++i)
    {
        Futile::Stopwatch stopwatch;
        stopwatch.start();
        WorkerPool pool("WorkerPool Test", cPoolSize[i]);
        for (size_t idx = 0; idx != cPoolSize[i]; ++idx)
        {
            pool.schedule(boost::bind(&WorkerPoolTest::BeBusy));
        }
        pool.interruptAndClearQueue();
        Assert(stopwatch.elapsedMs() - cSleepTime > -cMargin);
        Assert(stopwatch.elapsedMs() - cSleepTime < cMargin);
    }
}


TEST_F(WorkerPoolTest, Resize)
{
    // Test resize
    for (std::size_t i = 0; i < cPoolSizeCount; ++i)
    {
        Futile::Stopwatch stopwatch;
        stopwatch.start();
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

        stopwatch.stop();
        Assert(stopwatch.elapsedMs() - cSleepTime > -cMargin);
        Assert(stopwatch.elapsedMs() - cSleepTime < cMargin);
    }
}


TEST_F(WorkerPoolTest, JoinAll)
{
    // Test joinAll
    for (std::size_t i = 0; i < cPoolSizeCount; ++i)
    {
        Futile::Stopwatch stopwatch;
        stopwatch.start();
        WorkerPool pool("WorkerPool Test", cPoolSize[i]);
        for (size_t i = 0; i < pool.size(); ++i)
        {
            pool.schedule(boost::bind(&Futile::Sleep, 250));
        }
        pool.wait();
        stopwatch.stop();
    }
}


} // namespace testing
