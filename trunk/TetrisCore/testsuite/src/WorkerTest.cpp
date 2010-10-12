#include "WorkerTest.h"
#include "CppUnit/TestCaller.h"
#include "CppUnit/TestSuite.h"
#include "Tetris/Worker.h"
#include "Poco/Thread.h"
#include <boost/thread.hpp>
#include <iostream>


WorkerTest::WorkerTest(const std::string & inName):
    CppUnit::TestCase(inName),
    mRepeat(10)
{
}


WorkerTest::~WorkerTest()
{
}


void WorkerTest::printProgress(size_t a, size_t b)
{
    std::cout << (a + 1) << "/" << b << "\r";
}


void WorkerTest::CountTo(Poco::UInt64 inNumber)
{
    for (Poco::UInt64 idx = 0; idx != inNumber; ++idx)
    {
        boost::this_thread::interruption_point();
        Poco::Thread::sleep(10);
    }
}


void WorkerTest::testStatus()
{
    std::cout << "\n";
    for (size_t idx = 0; idx != mRepeat; ++idx)
    {
        printProgress(idx, mRepeat);
        testStatusImpl();
    }
    std::cout << "\n";
}


void WorkerTest::testStatusImpl()
{
    Tetris::Worker worker("TestWorker");
    assertEqual(worker.status(), Tetris::Worker::Status_Waiting);

    Poco::Thread::sleep(10);
    assertEqual(worker.status(), Tetris::Worker::Status_Waiting);

    worker.schedule(boost::bind(&WorkerTest::CountTo, 1000 * 1000));
    assertEqual(worker.status(), Tetris::Worker::Status_Waiting);

    Poco::Thread::sleep(10);
    assertEqual(worker.status(), Tetris::Worker::Status_Working);
    assertEqual(worker.size(), 0);
}


void WorkerTest::testSimpleInterrupt()
{
    std::cout << "\n";
    for (size_t idx = 0; idx != mRepeat; ++idx)
    {
        printProgress(idx, mRepeat);
        testSimpleInterruptImpl();
    }
    std::cout << "\n";
}


void WorkerTest::testSimpleInterruptImpl()
{
    Tetris::Worker worker("TestWorker");
    worker.interrupt();
    worker.schedule(boost::bind(&WorkerTest::CountTo, 1000 * 1000));
    Poco::Thread::sleep(10);
    assertEqual(worker.status(), Tetris::Worker::Status_Working);
    assertEqual(worker.size(), 0);
    
    worker.interrupt();
    Poco::Thread::sleep(10);
    assertEqual(worker.status(), Tetris::Worker::Status_Waiting);
    assertEqual(worker.size(), 0);
}


void WorkerTest::testAdvancedInterrupt()
{
    std::cout << "\n";
    for (size_t idx = 0; idx != mRepeat; ++idx)
    {
        printProgress(idx, mRepeat);
        testAdvancedInterruptImpl();
    }
    std::cout << "\n";
}


void WorkerTest::testAdvancedInterruptImpl()
{
    Tetris::Worker worker("TestWorker");
    worker.schedule(boost::bind(&WorkerTest::CountTo, 1000 * 1000));
    worker.schedule(boost::bind(&WorkerTest::CountTo, 1000 * 1000));
    Poco::Thread::sleep(10);
    assertEqual(worker.status(), Tetris::Worker::Status_Working);
    assertEqual(worker.size(), 1);

    worker.interrupt();
    Poco::Thread::sleep(10);
    assertEqual(worker.status(), Tetris::Worker::Status_Working);
    assertEqual(worker.size(), 0);

    worker.interrupt();
    Poco::Thread::sleep(10);
    assertEqual(worker.status(), Tetris::Worker::Status_Waiting);
    assertEqual(worker.size(), 0);

    worker.schedule(boost::bind(&WorkerTest::CountTo, 1000 * 1000));
    worker.schedule(boost::bind(&WorkerTest::CountTo, 1000 * 1000));
    worker.schedule(boost::bind(&WorkerTest::CountTo, 1000 * 1000));
    worker.schedule(boost::bind(&WorkerTest::CountTo, 1000 * 1000));
    worker.schedule(boost::bind(&WorkerTest::CountTo, 1000 * 1000));
    Poco::Thread::sleep(10);
    worker.interruptAndClearQueue();
    assertEqual(worker.status(), Tetris::Worker::Status_Waiting);
    assertEqual(worker.size(), 0);
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
	CppUnit_addTest(suite, WorkerTest, testStatus);
	CppUnit_addTest(suite, WorkerTest, testSimpleInterrupt);
	CppUnit_addTest(suite, WorkerTest, testAdvancedInterrupt);
	return suite;
}
