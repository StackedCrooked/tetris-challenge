#include "WorkerTest.h"
#include "Futile/Worker.h"
#include "Futile/WorkerPool.h"
#include "Futile/Threading.h"
#include "Futile/Assert.h"
#include "Futile/Stopwatch.h"
#include <iostream>


namespace testing {


using namespace Futile;


namespace {
static const UInt64 cSleepTimeMs = 100;
static const UInt64 cWaitTime = 100;
}


TEST_F(WorkerTest, Worker)
{
    Worker worker("TestWorker");
    ASSERT_EQ(worker.name(), "TestWorker");
    ASSERT_TRUE(worker.status() >= WorkerStatus_Idle);
    worker.wait();
    worker.interrupt();
    worker.interruptAndClearQueue();
    ASSERT_EQ(worker.size(), std::size_t(0));
}


TEST_F(WorkerTest, NoInterrupt)
{
    Worker worker("TestWorker");
    Futile::Stopwatch stopwatch;
    stopwatch.start();
    worker.schedule(boost::bind(&Futile::Sleep, cSleepTimeMs));
    worker.waitForStatus(WorkerStatus_Working);
    ASSERT_EQ(worker.size(), std::size_t(0));
    worker.wait();
    stopwatch.stop();
    ASSERT_LT(stopwatch.elapsedMs(), cSleepTimeMs + cWaitTime);
}


TEST_F(WorkerTest, WithInterrupt)
{
    Worker worker("TestWorker");
    Futile::Stopwatch stopwatch;
    stopwatch.start();
    worker.schedule(boost::bind(&WorkerTest::BeBusy));
    worker.waitForStatus(WorkerStatus_Working);
    worker.interrupt();
    ASSERT_EQ(worker.size(), std::size_t(0));
    stopwatch.stop();
    ASSERT_TRUE(stopwatch.elapsedMs() < cSleepTimeMs + cWaitTime);


    // Test with interrupt
    stopwatch.restart();
    worker.schedule(boost::bind(&WorkerTest::BeBusy));
    worker.schedule(boost::bind(&WorkerTest::BeBusy));
    worker.schedule(boost::bind(&WorkerTest::BeBusy));
    worker.schedule(boost::bind(&WorkerTest::BeBusy));
    worker.schedule(boost::bind(&WorkerTest::BeBusy));
    worker.waitForStatus(WorkerStatus_Working);
    ASSERT_EQ(worker.size(), std::size_t(4));
    worker.interrupt();
    ASSERT_TRUE(stopwatch.elapsedMs() < 2 * cSleepTimeMs);
    ASSERT_EQ(worker.size(), std::size_t(3));
    worker.interrupt();
    ASSERT_TRUE(stopwatch.elapsedMs() < 3 * cSleepTimeMs);
    ASSERT_EQ(worker.size(), std::size_t(2));
    worker.interruptAndClearQueue();
    ASSERT_EQ(worker.size(), std::size_t(0));
    stopwatch.stop();
    ASSERT_TRUE(stopwatch.elapsedMs() < 4 * cSleepTimeMs);

    // Interrupt twice should not crash.
    worker.interrupt();
    worker.interrupt();
    worker.interruptAndClearQueue();
    worker.interruptAndClearQueue();
}


} // namespace testing
