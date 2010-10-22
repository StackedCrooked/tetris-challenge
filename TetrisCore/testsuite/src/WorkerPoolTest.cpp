#include "WorkerPoolTest.h"
#include "CppUnit/TestCaller.h"
#include "CppUnit/TestSuite.h"
#include "Tetris/WorkerPool.h"
#include "Poco/Thread.h"
#include <boost/thread.hpp>
#include <iostream>


using namespace Tetris;


static const int cSleepTime = 100;


WorkerPoolTest::WorkerPoolTest(const std::string & inName):
    CppUnit::TestCase(inName)
{
}


WorkerPoolTest::~WorkerPoolTest()
{
}


void WorkerPoolTest::printProgress(size_t a, size_t b)
{
    std::cout << (a + 1) << "/" << b << "\r";
}


void WorkerPoolTest::BeBusy()
{
    while (true)
    {
        boost::this_thread::interruption_point();
        Poco::Thread::sleep(cSleepTime);
    }
}


void WorkerPoolTest::testWorkerPool()
{
    const int cPoolSize[] = {1, 2, 4, 8, 16, 32};
    const int cPoolSizeCount = sizeof(cPoolSize) / sizeof(cPoolSize[0]);

    // Test without interrupt
    for (int i = 0; i < cPoolSizeCount; ++i)
    {
        mStopwatch.restart();
        WorkerPool pool("WorkerPool Test", cPoolSize[i]);
        for (size_t idx = 0; idx != cPoolSize[i]; ++idx)
        {
            pool.getWorker()->schedule(boost::bind(&Poco::Thread::sleep, cSleepTime));
        }
        mStopwatch.stop();
        int overhead = static_cast<int>(mStopwatch.elapsed() / 1000) - cSleepTime;
        assert(overhead > -200);
        assert(overhead < 200);
    }

    // Test with interrupt
    for (int i = 0; i < cPoolSizeCount; ++i)
    {
        mStopwatch.restart();
        WorkerPool pool("WorkerPool Test", cPoolSize[i]);
        for (size_t idx = 0; idx != cPoolSize[i]; ++idx)
        {
            pool.getWorker()->schedule(boost::bind(&WorkerPoolTest::BeBusy));
        }
        pool.interruptAndClearQueue();
        mStopwatch.stop();
        int overhead = static_cast<int>(mStopwatch.elapsed() / 1000) - cSleepTime;
        assert(overhead > -200);
        assert(overhead < 200);
    }

    // Test setSize
    for (int i = 0; i < cPoolSizeCount; ++i)
    {
        mStopwatch.restart();
        WorkerPool pool("WorkerPool Test", cPoolSize[i]);
        for (size_t idx = 0; idx != cPoolSize[i]; ++idx)
        {
            pool.getWorker()->schedule(boost::bind(&WorkerPoolTest::BeBusy));
        }

        pool.setSize(cPoolSize[i]);
        assert(pool.size() == cPoolSize[i]);

        int newSize = static_cast<int>(0.5 + (cPoolSize[i] / 2.0));
        pool.setSize(newSize);
        assert(pool.size() == newSize);

        pool.setSize(0);
        assert(pool.size() == 0);

        mStopwatch.stop();
        int overhead = static_cast<int>(mStopwatch.elapsed() / 1000) - cSleepTime;
        assert(overhead > -200);
        assert(overhead < 200);
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
