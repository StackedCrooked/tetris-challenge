#include "WorkerPoolTest.h"
#include "CppUnit/TestCaller.h"
#include "CppUnit/TestSuite.h"
#include "Futile/Threading.h"
#include "Futile/WorkerPool.h"
#include "Futile/Assert.h"
#include "Poco/Thread.h"
#include <iostream>


using Futile::WorkerPool;


static const int cSleepTime = 100; // ms
static const int cMargin = 200; // ms


WorkerPoolTest::WorkerPoolTest(const std::string & inName):
    CppUnit::TestCase(inName),
    mIterationCount(20)
{
}


WorkerPoolTest::~WorkerPoolTest()
{
}


void WorkerPoolTest::BeBusy()
{
    while (true)
    {
        bool interruptRequest = boost::this_thread::interruption_requested();
        boost::this_thread::interruption_point();
        if (interruptRequest)
        {
            throw std::runtime_error("Interrupt requested!");
        }
        Poco::Thread::sleep(cSleepTime);
    }
}


void WorkerPoolTest::testWorkerPool()
{
    for (size_t idx = 0; idx < mIterationCount; ++idx)
    {
        std::cout << "\nTest workerpool iteration " << (idx + 1) << "/" << mIterationCount << std::flush << std::endl;
        testWorkerPoolImpl();
    }
}


void WorkerPoolTest::testWorkerPoolImpl()
{
    const std::size_t cPoolSize[] = {1, 2, 4, 8, 16, 32};
    const std::size_t cPoolSizeCount = sizeof(cPoolSize) / sizeof(cPoolSize[0]);

    // Test without interrupt
    for (std::size_t i = 0; i < cPoolSizeCount; ++i)
    {
        std::cout << "\r  Test without interrupt " << (i + 1) << "/" << cPoolSizeCount << std::flush;
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
    std::cout << std::endl;


    // Test with interrupt
    for (size_t i = 0; i < cPoolSizeCount; ++i)
    {
        std::cout << "\r  Test with interrupt    " << (i + 1) << "/" << cPoolSizeCount << std::flush;
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
    std::cout << std::endl;

    // Test resize
    for (int i = 0; i < cPoolSizeCount; ++i)
    {
        std::cout << "\r  Test resize            " << (i + 1) << "/" << cPoolSizeCount << std::flush;
        mStopwatch.restart();
        WorkerPool pool("WorkerPool Test", cPoolSize[i]);
        for (size_t idx = 0; idx != cPoolSize[i]; ++idx)
        {
            pool.schedule(boost::bind(&WorkerPoolTest::BeBusy));
        }

        pool.resize(cPoolSize[i]);
        Assert(pool.size() == cPoolSize[i]);

        int newSize = static_cast<int>(0.5 + (cPoolSize[i] / 2.0));
        pool.resize(newSize);
        Assert(pool.size() == newSize);

        pool.resize(0);
        Assert(pool.size() == 0);

        mStopwatch.stop();
        int elapsedMs = static_cast<int>(mStopwatch.elapsed() / 1000);
        Assert(elapsedMs - cSleepTime > -cMargin);
        Assert(elapsedMs - cSleepTime < cMargin);
    }
    std::cout << std::endl;

    // Test joinAll
    for (int i = 0; i < cPoolSizeCount; ++i)
    {
        std::cout << "\r  Test joinAll           " << (i + 1) << "/" << cPoolSizeCount << std::flush;
        mStopwatch.restart();
        WorkerPool pool("WorkerPool Test", cPoolSize[i]);
        for (size_t i = 0; i < pool.size(); ++i)
        {
            pool.schedule(boost::bind(&Poco::Thread::sleep, 250));
        }
        pool.wait();
        mStopwatch.stop();
    }
    std::cout << std::endl << std::flush;
}


void WorkerPoolTest::setUp()
{
}


void WorkerPoolTest::tearDown()
{
}


CppUnit::Test * WorkerPoolTest::suite()
{
    CppUnit::TestSuite * suite(new CppUnit::TestSuite("WorkerPoolTest"));
    CppUnit_addTest(suite, WorkerPoolTest, testWorkerPool);
    return suite;
}
