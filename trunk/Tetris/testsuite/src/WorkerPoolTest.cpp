#include "WorkerPoolTest.h"
#include "CppUnit/TestCaller.h"
#include "CppUnit/TestSuite.h"
#include "Tetris/Threading.h"
#include "Tetris/WorkerPool.h"
#include "Tetris/Assert.h"
#include "Poco/Thread.h"
#include <iostream>


using namespace Tetris;


static const int cSleepTime = 100;
static const int cMargin = 2000;


WorkerPoolTest::WorkerPoolTest(const std::string & inName):
    CppUnit::TestCase(inName),
    mIterationCount(10)
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
        std::cout << "Test workerpool iteration " << idx << std::flush << std::endl;
        testWorkerPoolImpl();
    }
}


void WorkerPoolTest::testWorkerPoolImpl()
{
    const size_t cPoolSize[] = {1, 2, 4, 8, 16};
    const size_t cPoolSizeCount = sizeof(cPoolSize) / sizeof(cPoolSize[0]);

    // Test without interrupt
    for (size_t i = 0; i < cPoolSizeCount; ++i)
    {
        mStopwatch.restart();
        WorkerPool pool("WorkerPool Test", cPoolSize[i]);
        for (size_t idx = 0; idx != cPoolSize[i]; ++idx)
        {
            pool.schedule(boost::bind(&Poco::Thread::sleep, cSleepTime));
        }
        mStopwatch.stop();
        Assert(static_cast<int>(mStopwatch.elapsed() / 1000) - cSleepTime > -cMargin);
        Assert(static_cast<int>(mStopwatch.elapsed() / 1000) - cSleepTime < cMargin);
    }

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
        mStopwatch.stop();
        Assert(static_cast<int>(mStopwatch.elapsed() / 1000) - cSleepTime > -cMargin);
        Assert(static_cast<int>(mStopwatch.elapsed() / 1000) - cSleepTime < cMargin);
    }

    // Test resize
    for (int i = 0; i < cPoolSizeCount; ++i)
    {
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
        Assert(static_cast<int>(mStopwatch.elapsed() / 1000) - cSleepTime > -cMargin);
        Assert(static_cast<int>(mStopwatch.elapsed() / 1000) - cSleepTime < cMargin);
    }

    // Test joinAll
    {
        mStopwatch.restart();
        WorkerPool pool("WorkerPool Test", 10);
        Assert(pool.size() == 10);
        for (size_t i = 0; i < pool.size(); ++i)
        {
            pool.schedule(boost::bind(&Poco::Thread::sleep, 1000));
        }
        pool.wait();
        mStopwatch.stop();
        Assert(pool.size() == 10);
        Assert(static_cast<int>(mStopwatch.elapsed() / 1000) - cSleepTime >= 0); // margin should be bigger than zero
        Assert(static_cast<int>(mStopwatch.elapsed() / 1000) - cSleepTime < cMargin);
    }
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
