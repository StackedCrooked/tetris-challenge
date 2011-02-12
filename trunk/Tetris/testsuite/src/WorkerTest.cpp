#include "WorkerTest.h"
#include "CppUnit/TestCaller.h"
#include "CppUnit/TestSuite.h"
#include "Tetris/Worker.h"
#include "Tetris/Assert.h"
#include "Poco/Thread.h"
#include "Poco/Stopwatch.h"
#include <boost/thread.hpp>
#include <iostream>


using namespace Tetris;


static const int cSleepTimeMs = 100;


WorkerTest::WorkerTest(const std::string & inName):
    CppUnit::TestCase(inName),
    mStopwatch(),
    mRepeat(10)
{
}


WorkerTest::~WorkerTest()
{
}


void WorkerTest::BeBusy()
{
    while (true)
    {
        boost::this_thread::interruption_point();
        Poco::Thread::sleep(cSleepTimeMs);
    }
}


void WorkerTest::testWorker()
{
    for (size_t idx = 0; idx != mRepeat; ++idx)
    {
        std::cout << "Test worker iteration " << idx << std::flush << std::endl;
        testWorkerImpl();
    }
}


void WorkerTest::testWorkerImpl()
{
    Worker worker("TestWorker");
    assertEqual(worker.name(), "TestWorker");
    Assert(worker.status() >= Worker::Status_Initial);
    worker.wait();
    worker.interrupt();
    worker.interruptAndClearQueue();
    assertEqual(worker.size(), 0);


    // Test without interrupt
    mStopwatch.restart();
    worker.schedule(boost::bind(&Poco::Thread::sleep, cSleepTimeMs));
    worker.waitForStatus(Worker::Status_Working);
    assertEqual(worker.size(), 0);
    worker.wait();
    mStopwatch.stop();
    Assert(mStopwatch.elapsed() / 1000.0 >= cSleepTimeMs - 100);
    Assert(mStopwatch.elapsed() / 1000.0 < 100 + cSleepTimeMs);


    // Test with interrupt
    mStopwatch.restart();
    worker.schedule(boost::bind(&WorkerTest::BeBusy));
    worker.waitForStatus(Worker::Status_Working);
    worker.interrupt();
    assertEqual(worker.size(), 0);
    mStopwatch.stop();
    Assert(mStopwatch.elapsed() / 1000.0 < 100 + cSleepTimeMs);


    // Test with interrupt
    mStopwatch.restart();
    worker.schedule(boost::bind(&WorkerTest::BeBusy));
    worker.schedule(boost::bind(&WorkerTest::BeBusy));
    worker.schedule(boost::bind(&WorkerTest::BeBusy));
    worker.schedule(boost::bind(&WorkerTest::BeBusy));
    worker.schedule(boost::bind(&WorkerTest::BeBusy));
    worker.waitForStatus(Worker::Status_Working);
    assertEqual(worker.size(), 4);
    worker.interrupt();
    Assert(mStopwatch.elapsed() / 1000 < 2 * cSleepTimeMs);
    assertEqual(worker.size(), 3);
    worker.interrupt();
    Assert(mStopwatch.elapsed() / 1000 < 3 * cSleepTimeMs);
    assertEqual(worker.size(), 2);
    worker.interruptAndClearQueue();
    assertEqual(worker.size(), 0);
    mStopwatch.stop();
    Assert(mStopwatch.elapsed() / 1000 < 4 * cSleepTimeMs);

    // Interrupt twice should not crash.
    worker.interrupt();
    worker.interrupt();
    worker.interruptAndClearQueue();
    worker.interruptAndClearQueue();
}


void WorkerTest::setUp()
{
}


void WorkerTest::tearDown()
{
}


CppUnit::Test * WorkerTest::suite()
{
    CppUnit::TestSuite * suite(new CppUnit::TestSuite("WorkerTest"));
    CppUnit_addTest(suite, WorkerTest, testWorker);
    return suite;
}
