#include "WorkerTest.h"
#include "Futile/Worker.h"
#include "Futile/Threading.h"
#include "Futile/Assert.h"
#include "Poco/Thread.h"
#include "Poco/Stopwatch.h"
#include <iostream>


namespace testing {


static const int cSleepTimeMs = 100;


WorkerTest::WorkerTest():
    TetrisTest(),
    mRepeat(10)
{
}


TEST_F(WorkerTest, Worker)
{
    Worker worker("TestWorker");
    ASSERT_EQ(worker.name(), "TestWorker");
    ASSERT_TRUE(worker.status() >= WorkerStatus_Idle);
    worker.wait();
    worker.interrupt();
    worker.interruptAndClearQueue();
    ASSERT_EQ(worker.size(), 0);


    // Test without interrupt
    mStopwatch.restart();
    worker.schedule(boost::bind(&Futile::Sleep, cSleepTimeMs));
    worker.waitForStatus(WorkerStatus_Working);
    ASSERT_EQ(worker.size(), 0);
    worker.wait();
    mStopwatch.stop();
    ASSERT_TRUE(mStopwatch.elapsed() / 1000.0 >= cSleepTimeMs - 100);
    ASSERT_TRUE(mStopwatch.elapsed() / 1000.0 < 100 + cSleepTimeMs);


    // Test with interrupt
    mStopwatch.restart();
    worker.schedule(boost::bind(&WorkerTest::BeBusy));
    worker.waitForStatus(WorkerStatus_Working);
    worker.interrupt();
    ASSERT_EQ(worker.size(), 0);
    mStopwatch.stop();
    ASSERT_TRUE(mStopwatch.elapsed() / 1000.0 < 100 + cSleepTimeMs);


    // Test with interrupt
    mStopwatch.restart();
    worker.schedule(boost::bind(&WorkerTest::BeBusy));
    worker.schedule(boost::bind(&WorkerTest::BeBusy));
    worker.schedule(boost::bind(&WorkerTest::BeBusy));
    worker.schedule(boost::bind(&WorkerTest::BeBusy));
    worker.schedule(boost::bind(&WorkerTest::BeBusy));
    worker.waitForStatus(WorkerStatus_Working);
    ASSERT_EQ(worker.size(), 4);
    worker.interrupt();
    ASSERT_TRUE(mStopwatch.elapsed() / 1000 < 2 * cSleepTimeMs);
    ASSERT_EQ(worker.size(), 3);
    worker.interrupt();
    ASSERT_TRUE(mStopwatch.elapsed() / 1000 < 3 * cSleepTimeMs);
    ASSERT_EQ(worker.size(), 2);
    worker.interruptAndClearQueue();
    ASSERT_EQ(worker.size(), 0);
    mStopwatch.stop();
    ASSERT_TRUE(mStopwatch.elapsed() / 1000 < 4 * cSleepTimeMs);

    // Interrupt twice should not crash.
    worker.interrupt();
    worker.interrupt();
    worker.interruptAndClearQueue();
    worker.interruptAndClearQueue();
}


} // namespace testing
